#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

// 미리보기 공 업데이트 함수
void UFruitTrajectoryHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 접시 위치 찾기
    FVector PlateCenter = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
    }

    // 이전 미리보기 공 제거
    if (Controller->PreviewBall)
    {
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 플레이어 카메라 정보 가져오기
    APawn* PlayerPawn = Controller->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: PlayerPawn이 NULL입니다!"));
        return;
    }
    
    // 플레이어 방향 계산
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerDirection = PlayerPawn->GetActorForwardVector();
    
    // 접시보다 50.f 높게 설정된 위치 계산
    FVector PreviewLocation = FVector(
        PlayerLocation.X + PlayerDirection.X * 300.f,
        PlayerLocation.Y + PlayerDirection.Y * 300.f,
        PlateCenter.Z + 50.f  // 접시보다 50.f 높게 설정
    );
    
    // 공통 함수 호출로 공 생성 (물리 비활성화)
    Controller->PreviewBall = UFruitThrowHelper::SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
    
    if (Controller->PreviewBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성 성공 - 위치: %s"), *PreviewLocation.ToString());
        
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
        
    // 기존 궤적 디버그 정보 제거 (새로운 궤적을 그리기 전에)
    FlushPersistentDebugLines(Controller->GetWorld());
    FlushDebugStrings(Controller->GetWorld());
        
    // 접시까지의 벡터 계산
    FVector ToPlateCenterExact = TargetLocation - StartLocation;
    float ExactDistance = ToPlateCenterExact.Size();
    
    // 수평 거리 계산
    FVector HorizontalDist = ToPlateCenterExact;
    HorizontalDist.Z = 0;
    float HorizontalDistance = HorizontalDist.Size();
    
    // 발사 방향과 힘 계산 (실제 발사 코드와 동일한 로직)
    float HeightFactor;
    if (HorizontalDistance < 200.0f)
        HeightFactor = 1.2f;
    else if (HorizontalDistance < 400.0f)
        HeightFactor = 1.0f;
    else if (HorizontalDistance < 600.0f)
        HeightFactor = 0.8f;
    else
        HeightFactor = 0.6f;
    
    FVector HorizontalDir = HorizontalDist.GetSafeNormal();
    FVector LaunchDirection = HorizontalDir + FVector(0, 0, HeightFactor);
    LaunchDirection.Normalize();
    
    float ForceMultiplier;
    if (HorizontalDistance < 200.0f)
        ForceMultiplier = 0.5f;
    else if (HorizontalDistance < 400.0f)
        ForceMultiplier = 0.8f;
    else if (HorizontalDistance < 600.0f)
        ForceMultiplier = 1.0f;
    else
        ForceMultiplier = 1.2f;
    
    // 최종 힘과 초기 속도 계산
    float AdjustedForce = Controller->ThrowForce * ForceMultiplier;
    
    // 질량 가져오기 - 물리 시뮬레이션 및 콜리전 설정 변경 필요
    float Mass = 1.0f; // 기본 질량값
    
    UPrimitiveComponent* PrimComp = nullptr;
    if (Controller->PreviewBall)
    {
        PrimComp = Cast<UPrimitiveComponent>(
            Controller->PreviewBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp)
        {
            // 원래 물리 및 콜리전 상태 저장
            bool bWasSimulatingPhysics = PrimComp->IsSimulatingPhysics();
            ECollisionEnabled::Type OldCollisionType = PrimComp->GetCollisionEnabled();
            
            // 일시적으로 물리 시뮬레이션 및 콜리전 활성화
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);
            
            // 활성화 후 질량 가져오기
            Mass = PrimComp->GetMass();
            
            // 원래 상태로 되돌리기
            PrimComp->SetSimulatePhysics(bWasSimulatingPhysics);
            PrimComp->SetCollisionEnabled(OldCollisionType);
            
            UE_LOG(LogTemp, Log, TEXT("궤적 계산을 위한 공 질량: %f"), Mass);
        }
    }
    
    // 초기 속도 계산
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / Mass);
    
    // 중력 가속도 (UE4 기본값은 약 -980 cm/s^2)
    float GravityZ = GetDefault<UPhysicsSettings>()->DefaultGravityZ;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 궤적 계산 - 포물선 운동 방정식 사용
    TArray<FVector> TrajectoryPoints;
    const int32 NumPoints = 30; // 궤적 포인트 수
    const float TimeInterval = 0.066f; // 약 15fps에 해당하는 시간 간격
    const float MaxTime = 3.0f; // 최대 3초까지 시뮬레이션
    
    TrajectoryPoints.Add(StartLocation); // 시작점 추가
    
    // 시간 간격으로 위치 계산
    for (float Time = TimeInterval; Time < MaxTime; Time += TimeInterval)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 바닥 아래로 가지 않도록 체크
        if (Position.Z < 0.0f)
            break;
            
        TrajectoryPoints.Add(Position);
        
        // 목표 지점 부근에 도달했는지 체크
        float DistanceToTarget = FVector::Dist(Position, TargetLocation);
        if (DistanceToTarget < 10.0f)
            break;
    }
    
    // 계산된 궤적 그리기
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 각 점에 작은 구체 그리기
        DrawDebugSphere(
            Controller->GetWorld(),
            TrajectoryPoints[i],
            5.0f,  // 작은 크기의 구체
            8,
            FColor::Yellow,
            false,
            0.1f,  // 0.1초 유지
            0,
            1.0f
        );
        
        // 점들을 선으로 연결
        DrawDebugLine(
            Controller->GetWorld(),
            TrajectoryPoints[i],
            TrajectoryPoints[i + 1],
            FColor::Yellow,
            false,
            0.1f,
            0,
            1.0f
        );
    }
    
    // 목표 지점에 도달 위치 표시
    if (TrajectoryPoints.Num() > 1)
    {
        DrawDebugSphere(
            Controller->GetWorld(),
            TrajectoryPoints.Last(),
            10.0f,  // 도착 지점은 약간 더 크게
            8,
            FColor::Green,
            false,
            0.1f,
            0,
            2.0f
        );
    }
}