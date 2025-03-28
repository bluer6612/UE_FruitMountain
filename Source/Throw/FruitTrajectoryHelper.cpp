#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h" // 새 헬퍼 클래스 포함
#include "FruitSpawnHelper.h" // 공의 질량 계산을 위한 헬퍼 클래스 포함
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

// 궤적 생성 및 그리기를 위한 공통 함수
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(
    AFruitPlayerController* Controller,
    const FVector& StartLocation,
    const FVector& TargetLocation,
    float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
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
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / BallMass);
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산
    const float TimeStep = 0.05f; // 더 촘촘하게 포인트 생성
    const float MaxTime = 5.0f;
    
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
        if (DistanceToTarget < 20.0f)
            break;
    }
    
    return TrajectoryPoints;
}

// 최적화된 궤적 시각화 함수 - 개선 버전 (인자 한 줄로 정리)
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 선 그리기 설정
    float Duration = bPersistent ? -1.0f : 0.1f; // 임시 궤적은 더 짧게 유지
    
    // 색상 설정
    FColor LineColor = bPersistent ? FColor(0, 120, 255) : FColor(0, 200, 255, 200);
    FColor PointColor = bPersistent ? FColor(0, 120, 255) : FColor(0, 180, 255, 180);
    FColor EndColor = bPersistent ? FColor(0, 255, 0) : FColor(0, 255, 120, 200);
    
    // 선 두께 및 점 크기
    float LineThickness = bPersistent ? 2.5f : 2.0f;
    float PointSize = bPersistent ? 6.0f : 5.0f;
    float EndPointSize = bPersistent ? 12.0f : 10.0f;
    
    // 점 간격 - 영구적 궤적은 더 많은 점 표시
    int32 PointInterval = bPersistent ? 3 : 5;
    
    // 새 궤적 그리기
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 점들을 선으로 연결 (인자 한 줄로)
        DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, bPersistent, Duration, TrajectoryID, LineThickness);
        
        // 일정 간격으로 구체 그리기
        if (i % PointInterval == 0)
        {
            // 인자 한 줄로 정리
            DrawDebugSphere(World, TrajectoryPoints[i], PointSize, 8, PointColor, bPersistent, Duration, TrajectoryID, 1.0f);
        }
    }
    
    // 마지막 지점은 큰 구체로 표시 (인자 한 줄로)
    if (TrajectoryPoints.Num() > 1)
    {
        DrawDebugSphere(World, TrajectoryPoints.Last(), EndPointSize, 12, EndColor, bPersistent, Duration, TrajectoryID, 1.0f);
    }
}

// 통합된 함수들을 사용하는 업데이트 궤적 함수 - 개선 버전
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 기존 모든 궤적 제거 - 궤적 ID 관리 대신 FlushDebugStrings 사용
    FlushDebugStrings(World);
    
    // 공의 질량 계산 - 공의 종류에 따라
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 시작점 조정 - 공의 중심으로 수정
    FVector AdjustedStartLocation = StartLocation;
    
    // 공이 있으면 공의 실제 중심 위치 사용
    if (Controller->PreviewBall)
    {
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            Controller->PreviewBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            // 메시의 바운드 정보 가져오기
            FVector Origin;
            FVector BoxExtent;
            MeshComp->GetLocalBounds(Origin, BoxExtent);
            
            // 공의 중심점 계산 - 공의 Origin 위치에 중심점 오프셋 적용
            AdjustedStartLocation = Controller->PreviewBall->GetActorLocation() + Origin;
            
            UE_LOG(LogTemp, Verbose, TEXT("공 중심 조정: 원래=%s, 조정=%s, 오프셋=%s"), 
                *StartLocation.ToString(), *AdjustedStartLocation.ToString(), *Origin.ToString());
        }
    }
    
    // 궤적 계산 - 조정된 시작점 사용
    TArray<FVector> NewTrajectoryPoints = CalculateTrajectoryPoints(
        Controller, AdjustedStartLocation, TargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (NewTrajectoryPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("궤적 생성 실패: 충분한 포인트가 생성되지 않음"));
        return;
    }
    
    // 새 궤적 그리기 - 기존 ID 관리 대신 고정 ID 사용
    const int32 FixedTrajectoryID = 9999; // 항상 같은 ID 사용
    DrawTrajectoryPath(World, NewTrajectoryPoints, TargetLocation, false, FixedTrajectoryID);
    
    UE_LOG(LogTemp, Verbose, TEXT("궤적 업데이트: %d개 포인트 생성, 시작=%s, 목표=%s"), 
        NewTrajectoryPoints.Num(), *AdjustedStartLocation.ToString(), *TargetLocation.ToString());
}

// 통합된 함수들을 사용하는 영구 궤적 그리기 함수
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 이전 영구 디버그 라인 제거
    FlushPersistentDebugLines(World);
    
    // 공의 질량 계산 - 공의 종류에 따라
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 궤적 계산
    TArray<FVector> TrajectoryPoints = CalculateTrajectoryPoints(
        Controller, StartLocation, TargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (TrajectoryPoints.Num() < 2) {
        UE_LOG(LogTemp, Error, TEXT("영구 궤적 생성 실패: 충분한 포인트가 생성되지 않음"));
        return;
    }
    
    // 영구 궤적 그리기 (TrajectoryID=0, bPersistent=true)
    DrawTrajectoryPath(World, TrajectoryPoints, TargetLocation, true, 0);
    
    UE_LOG(LogTemp, Warning, TEXT("영구 궤적 그리기: %d개 포인트 생성"), TrajectoryPoints.Num());
}