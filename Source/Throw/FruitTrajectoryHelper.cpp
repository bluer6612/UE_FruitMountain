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

// 궤적 생성 및 그리기를 위한 공통 함수
void UFruitTrajectoryHelper::CalculateTrajectoryPoints(
    AFruitPlayerController* Controller,
    const FVector& StartLocation,
    const FVector& TargetLocation,
    TArray<FVector>& OutTrajectoryPoints)
{
    if (!Controller)
        return;
        
    // 초기화
    OutTrajectoryPoints.Empty();
    
    // 공의 질량 계산
    float BallMass = 10.0f; // 기본값
    
    // 공이 있으면 질량 가져오기
    if (Controller->PreviewBall)
    {
        // StaticMeshComponent에서 질량 가져오기
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            Controller->PreviewBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            BallMass = MeshComp->GetMass();
        }
        else
        {
            // 크기 기반 질량 예상 계산 (컴포넌트가 없는 경우)
            float BaseSize = 0.5f;
            float ScaleFactor = 1.f + 0.1f * (Controller->CurrentBallType - 1);
            float BallSize = BaseSize * ScaleFactor;
            
            // 질량은 부피에 비례함 (크기의 세제곱)
            BallMass = 10.0f * FMath::Pow(BallSize, 3.0f) / FMath::Pow(0.5f, 3.0f);
        }
    }
    
    // 공통 함수 호출하여 던지기 파라미터 계산
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(
        Controller,
        StartLocation,
        TargetLocation,
        AdjustedForce,
        LaunchDirection,
        BallMass);
    
    // 초기 속도 계산
    FVector InitialVelocity = LaunchDirection * AdjustedForce;
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    OutTrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산
    const float TimeStep = 0.1f;
    const float MaxTime = 5.0f;
    
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 포인트 추가
        OutTrajectoryPoints.Add(Position);
        
        // 바닥에 닿았는지 확인
        if (Position.Z < 0.0f)
            break;
            
        // 목표 지점 근처인지 확인
        float DistanceToTarget = FVector::Dist(Position, TargetLocation);
        if (DistanceToTarget < 20.0f)
            break;
    }
}

// 최적화된 궤적 업데이트 함수
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 정적 변수로 이전 궤적 정보 유지
    static TArray<FVector> LastTrajectoryPoints;
    static int32 TrajectoryID = 0;
    
    // 이전 궤적 제거
    if (LastTrajectoryPoints.Num() > 0) {
        for (int32 i = 0; i < LastTrajectoryPoints.Num() - 1; i++) {
            // 이전 라인 제거
            DrawDebugLine(
                World,
                LastTrajectoryPoints[i],
                LastTrajectoryPoints[i + 1],
                FColor::Black,
                false,
                0.0f,
                TrajectoryID,
                0.0f
            );
            
            // 이전 구체 제거
            if (i % 5 == 0) {
                DrawDebugSphere(
                    World,
                    LastTrajectoryPoints[i],
                    5.0f,
                    8,
                    FColor::Black,
                    false,
                    0.0f,
                    TrajectoryID,
                    0.0f
                );
            }
        }
    }
    
    // 궤적 ID 업데이트
    TrajectoryID++;
    
    // 공통 함수를 통해 궤적 점들 계산
    TArray<FVector> NewTrajectoryPoints;
    CalculateTrajectoryPoints(Controller, StartLocation, TargetLocation, NewTrajectoryPoints);
    
    // 새 궤적 그리기
    for (int32 i = 0; i < NewTrajectoryPoints.Num() - 1; i++)
    {
        // 점들을 선으로 연결
        DrawDebugLine(
            World,
            NewTrajectoryPoints[i],
            NewTrajectoryPoints[i + 1],
            FColor::Blue,
            false, // 임시 표시
            0.1f,  // 다음 프레임까지 유지
            TrajectoryID, // 고유 ID 사용
            2.0f
        );
        
        // 5개 점마다 구체 그리기
        if (i % 5 == 0)
        {
            DrawDebugSphere(
                World,
                NewTrajectoryPoints[i],
                5.0f,
                8,
                FColor::Blue,
                false,
                0.1f,
                TrajectoryID,
                1.0f
            );
        }
    }
    
    // 마지막 지점은 큰 구체로 표시
    if (NewTrajectoryPoints.Num() > 1)
    {
        DrawDebugSphere(
            World,
            NewTrajectoryPoints.Last(),
            10.0f,
            12,
            FColor::Green,
            false,
            0.1f,
            TrajectoryID,
            1.0f
        );
    }
    
    // 실제 목표 지점도 표시
    DrawDebugSphere(
        World,
        TargetLocation,
        15.0f,
        12,
        FColor::Blue,
        false,
        0.1f,
        TrajectoryID,
        1.0f
    );
    
    // 현재 궤적 저장
    LastTrajectoryPoints = NewTrajectoryPoints;
}

// 간소화된 DrawTrajectoryPath 함수
void UFruitTrajectoryHelper::DrawTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 이전 영구 디버그 라인 제거
    FlushPersistentDebugLines(World);
    
    // 공통 함수를 통해 궤적 점들 계산
    TArray<FVector> TrajectoryPoints;
    CalculateTrajectoryPoints(Controller, StartLocation, TargetLocation, TrajectoryPoints);
    
    // 계산된 궤적 그리기 - 영구적으로 표시
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 점들을 선으로 연결
        DrawDebugLine(
            World,
            TrajectoryPoints[i],
            TrajectoryPoints[i + 1],
            FColor::Blue,
            true,
            -1.0f,
            0,
            2.0f
        );
        
        // 5개 점마다 구체 그리기
        if (i % 5 == 0)
        {
            DrawDebugSphere(
                World,
                TrajectoryPoints[i],
                5.0f,
                8,
                FColor::Blue,
                true,
                -1.0f,
                0,
                1.0f
            );
        }
    }
    
    // 마지막 지점과 목표 지점 표시
    if (TrajectoryPoints.Num() > 1)
    {
        DrawDebugSphere(
            World,
            TrajectoryPoints.Last(),
            10.0f,
            12,
            FColor::Green,
            true,
            -1.0f,
            0,
            1.0f
        );
    }
    
    // 실제 목표 지점도 표시
    DrawDebugSphere(
        World,
        TargetLocation,
        15.0f,
        12,
        FColor::Blue,
        true,
        -1.0f,
        0,
        1.0f
    );
}