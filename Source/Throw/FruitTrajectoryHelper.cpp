#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h" // 새 헬퍼 클래스 포함
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

// 미리보기 공 업데이트 함수 효율적으로 개선
void UFruitTrajectoryHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 공통 함수를 사용하여 위치 계산 (카메라 각도 전달)
    FVector PreviewLocation = UFruitThrowHelper::CalculatePlateEdgeSpawnPosition(
        Controller->GetWorld(), 50.f, Controller->CameraOrbitAngle);
    
    if (PreviewLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: 유효한 위치를 계산할 수 없습니다!"));
        return;
    }
    
    // 미리보기 공이 없는 경우에만 새로 생성
    if (!Controller->PreviewBall)
    {
        // 공통 함수 호출로 공 생성 (물리 비활성화)
        Controller->PreviewBall = UFruitThrowHelper::SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 새로 생성 - 위치: %s"), *PreviewLocation.ToString());
    }
    else
    {
        // 기존 공의 위치만 업데이트
        Controller->PreviewBall->SetActorLocation(PreviewLocation);
        
        // 필요한 경우 크기도 업데이트
        float BaseSize = 0.5f;
        float ScaleFactor = 1.f + 0.1f * (Controller->CurrentBallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        Controller->PreviewBall->SetActorScale3D(FVector(BallSize));
        
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 위치 업데이트 - 위치: %s"), *PreviewLocation.ToString());
    }
    
    if (Controller->PreviewBall)
    {
        // 접시 위치 찾기
        FVector PlateCenter = FVector::ZeroVector;
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
        if (PlateActors.Num() > 0)
        {
            PlateCenter = PlateActors[0]->GetActorLocation();
        }
        
        // 예상 경로 계산 및 표시
        DrawTrajectoryPath(Controller, PreviewLocation, PlateCenter);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 공 생성에 실패했습니다!"));
    }
}

// 예상 궤적 계산 및 그리기 함수
void UFruitTrajectoryHelper::DrawTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 이전 영구 디버그 라인 제거
    FlushPersistentDebugLines(World);
    
    // 접시까지의 벡터 계산
    FVector ToPlateCenterExact = TargetLocation - StartLocation;
    float ExactDistance = ToPlateCenterExact.Size();
    
    // 수평 거리 계산
    FVector HorizontalDist = ToPlateCenterExact;
    HorizontalDist.Z = 0;
    float HorizontalDistance = HorizontalDist.Size();
    
    // UE_LOG(LogTemp, Warning, TEXT("궤적 계산 - 시작: %s, 목표: %s, 거리: %f"), 
    //     *StartLocation.ToString(), *TargetLocation.ToString(), HorizontalDistance);
    
    // 공통 함수 호출하여 던지기 파라미터 계산
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(
        Controller,
        StartLocation,
        TargetLocation,
        AdjustedForce,
        LaunchDirection);
    
    // 초기 속도 계산
    FVector InitialVelocity = LaunchDirection * AdjustedForce;
    
    // 중력 가속도 (UE4 기본값은 약 -980 cm/s^2)
    float GravityZ = -980.0f; // 기본값 하드코딩
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // UE_LOG(LogTemp, Warning, TEXT("궤적 계산 - 방향: %s, 힘: %f, 초기속도: %s"), 
    //     *LaunchDirection.ToString(), AdjustedForce, *InitialVelocity.ToString());
    
    // 궤적 계산 - 포물선 운동 방정식 사용
    TArray<FVector> TrajectoryPoints;
    const float TimeStep = 0.1f; // 더 긴 간격으로 변경
    const float MaxTime = 5.0f; // 더 긴 시간 동안 시뮬레이션
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 포인트 추가
        TrajectoryPoints.Add(Position);
        
        // 바닥에 닿았는지 확인
        if (Position.Z < 0.0f)
            break;
            
        // 목표 지점 근처인지 확인
        float DistanceToTarget = FVector::Dist(Position, TargetLocation);
        if (DistanceToTarget < 20.0f) // 더 관대한 범위
            break;
    }
    
    // 계산된 궤적 그리기 - 영구적으로 표시하도록 변경
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 점들을 선으로 연결 - 영구적으로 표시(true로 변경)
        DrawDebugLine(
            World,
            TrajectoryPoints[i],
            TrajectoryPoints[i + 1],
            FColor::Blue,
            true, // false -> true로 변경 (영구적으로 표시)
            -1.0f, // 타임아웃을 -1로 설정하여 무한대로 유지
            0,
            2.0f // 두꺼운 선으로 표시
        );
        
        // 5개 점마다 구체 그리기 - 영구적으로 표시
        if (i % 5 == 0)
        {
            DrawDebugSphere(
                World,
                TrajectoryPoints[i],
                5.0f, // 작은 구체
                8,
                FColor::Blue,
                true, // false -> true로 변경 (영구적으로 표시)
                -1.0f, // 타임아웃을 -1로 설정하여 무한대로 유지
                0,
                1.0f
            );
        }
    }
    
    // 마지막 지점은 큰 구체로 표시 - 영구적으로 표시
    if (TrajectoryPoints.Num() > 1)
    {
        DrawDebugSphere(
            World,
            TrajectoryPoints.Last(),
            10.0f, // 더 큰 구체
            12,
            FColor::Green,
            true, // false -> true로 변경 (영구적으로 표시)
            -1.0f, // 타임아웃을 -1로 설정하여 무한대로 유지
            0,
            1.0f
        );
    }
    
    // 실제 목표 지점도 표시 - 영구적으로 표시
    DrawDebugSphere(
        World,
        TargetLocation,
        15.0f, // 큰 구체
        12,
        FColor::Blue,
        true, // false -> true로 변경 (영구적으로 표시)
        -1.0f, // 타임아웃을 -1로 설정하여 무한대로 유지
        0,
        1.0f
    );
}