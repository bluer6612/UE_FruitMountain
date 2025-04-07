#include "FruitTrajectoryHelper.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h"
#include "Gameplay/Create/FruitSpawnHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Actors/FruitBall.h"

TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    UWorld* World = Controller->GetWorld();
    
    // 1. 각도 가져오기
    float UseAngle = Controller->ThrowAngle;
    float MinAngle, MaxAngle;
    UFruitPhysicsHelper::GetThrowAngleRange(MinAngle, MaxAngle);
    UseAngle = FMath::Clamp(UseAngle, MinAngle, MaxAngle);
    
    // 2. 물리 헬퍼를 통해 조정된 타겟 위치 계산
    FVector PlateCenter;
    float PlateTopHeight;
    FVector AdjustedTarget = UFruitPhysicsHelper::CalculateAdjustedTargetLocation(
        World, StartLocation, TargetLocation, UseAngle, PlateCenter, PlateTopHeight);
    
    // 3. 물리 헬퍼를 통해 초기 속도 계산
    FVector LaunchVelocity;
    bool bSuccess = UFruitPhysicsHelper::CalculateThrowVelocity(
        StartLocation, AdjustedTarget, UseAngle, BallMass, LaunchVelocity);
    
    if (!bSuccess)
    {
        // 계산 실패 시 기본값 사용
        UE_LOG(LogTemp, Warning, TEXT("물리 계산에 실패하여 기본 속도 사용"));
        
        // 기본 방향과 속도 설정 (물리 헬퍼의 함수 사용)
        float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
        FVector HorizontalDelta = TargetLocation - StartLocation;
        FVector HorizontalDir = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).GetSafeNormal();
        
        FVector LaunchDirection = FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), 
                                          HorizontalDir.Y * FMath::Cos(ThrowAngleRad), 
                                          FMath::Sin(ThrowAngleRad)).GetSafeNormal();
        
        LaunchVelocity = LaunchDirection * 300.0f; // 기본 속도
    }
    
    // 4. 시간에 따른 포물선 궤적 계산
    float Gravity = 980.0f; // 중력 가속도
    const float TimeStep = 0.05f;
    const float MaxTime = 5.0f;
    FVector Gravity3D = FVector(0, 0, -Gravity);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간에 따른 위치 계산 (포물선 방정식)
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        FVector Position = StartLocation + LaunchVelocity * Time + 0.5f * Gravity3D * Time * Time;
        TrajectoryPoints.Add(Position);
        
        // 지면에 닿았거나 목표 근처에 도달했는지 확인
        if (Position.Z <= 0.0f || FVector::Dist(Position, AdjustedTarget) < 10.0f)
            break;
    }
    
    // UE_LOG(LogTemp, Log, TEXT("포물선 계산: 각도=%.1f, 속도=%.1f, 포인트=%d개"),
    //     UseAngle, LaunchVelocity.Size(), TrajectoryPoints.Num());
    
    return TrajectoryPoints;
}

void UFruitTrajectoryHelper::UpdateTrajectoryPath(
    AFruitPlayerController* Controller, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    bool bPersistent, 
    int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;

    UWorld* World = Controller->GetWorld();
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;

    // 기존 궤적 비우기
    FlushPersistentDebugLines(World);

    // 1. 현재 각도 가져오기
    float MinAngle, MaxAngle;
    UFruitPhysicsHelper::GetThrowAngleRange(MinAngle, MaxAngle);
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, MinAngle, MaxAngle);

    // 2. 물리 헬퍼를 통해 조정된 타겟 위치 계산
    FVector PlateCenter;
    float PlateTopHeight;
    FVector AdjustedTarget = UFruitPhysicsHelper::CalculateAdjustedTargetLocation(
        World, StartLocation, TargetLocation, UseAngle, PlateCenter, PlateTopHeight);

    // 3. 궤적 계산 및 시각화
    float BallMass = AFruitBall::CalculateBallMass(Controller->CurrentBallType);
    TArray<FVector> TrajectoryPoints = CalculateTrajectoryPoints(Controller, StartLocation, AdjustedTarget, BallMass);
    DrawTrajectoryPath(World, TrajectoryPoints, TrajectoryID);
}

// 베지어 곡선으로 포물선 포인트 계산
TArray<FVector> UFruitTrajectoryHelper::CalculateBezierPoints(const FVector& Start, const FVector& End, float PeakHeight, int32 PointCount)
{
    TArray<FVector> Points;
    Points.Reserve(PointCount);
    
    // 정점 위치 계산 - 접시 크기 변화에 맞게 조정
    FVector HorizontalDelta = End - Start;
    // 더 높은 정밀도를 위해 비율 조정: 0.5f → 0.45f
    FVector Peak = Start + HorizontalDelta * 0.45f;
    
    // 피크 높이를 약간 높게 조정 (20% 증가)
    Peak.Z = FMath::Max(Start.Z, End.Z) + PeakHeight * 1.2f;
    
    // 베지어 곡선 포인트 생성
    for (int32 i = 0; i < PointCount; i++)
    {
        float t = (float)i / (PointCount - 1);
        FVector Point = FMath::Pow(1.0f - t, 2) * Start + 2 * t * (1.0f - t) * Peak + t * t * End;
        Points.Add(Point);
    }
    
    return Points;
}

// 궤적 시각화 - 통합된 버전
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID)
{
    if (!World || Points.Num() < 2)
    {
        return;
    }
        
    FColor PathColor = FColor(135, 206, 235, 180);
    bool bClearExisting = true;
    int32 MarkerCount = 3;
    float LineThickness = 1.2f;
    
    // 필요한 경우 기존 궤적 비우기
    if (bClearExisting)
    {
        FlushPersistentDebugLines(World);
    }
    
    // 모든 포인트를 연결하는 선 그리기
    for (int32 i = 0; i < Points.Num() - 1; i++)
    {
        DrawDebugLine(World, Points[i], Points[i + 1], PathColor, true, -1.0f, TrajectoryID, LineThickness);
    }
    
    // 포물선 상의 주요 지점에만 마커 표시
    if (Points.Num() >= MarkerCount)
    {
        for (int32 i = 0; i < MarkerCount; i++)
        {
            int32 Index = (i * (Points.Num() - 1)) / (MarkerCount - 1);
            DrawDebugBox(World, Points[Index], FVector(0.01f), FQuat::Identity, PathColor, true, -1.0f, TrajectoryID);
        }
    }
}