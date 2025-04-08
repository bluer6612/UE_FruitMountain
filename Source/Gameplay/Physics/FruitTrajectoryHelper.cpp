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

void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 기존 궤적 비우기
    FlushPersistentDebugLines(World);
    
    // 1. 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 2. 물리 계산 결과 가져오기
    FThrowPhysicsResult PhysicsResult = UFruitPhysicsHelper::CalculateThrowPhysics(
        World, StartLocation, TargetLocation, Controller->ThrowAngle, BallMass);
    
    // 3. 궤적 계산
    TArray<FVector> TrajectoryPoints;
    
    bool useBezier = false;
    if (useBezier)
    {
        // 베지어 곡선 방식
        TrajectoryPoints = UFruitPhysicsHelper::CalculateBezierPoints(
            StartLocation, PhysicsResult.AdjustedTarget, PhysicsResult.PeakHeight, 19);
    }
    else
    {
        // 물리 기반 궤적 (FruitPhysicsHelper로 직접 호출)
        TrajectoryPoints = UFruitPhysicsHelper::CalculateTrajectoryPoints(
            World, StartLocation, TargetLocation, Controller->ThrowAngle, BallMass);
    }
    
    // 4. 궤적 시각화
    DrawTrajectoryPath(World, TrajectoryPoints, TrajectoryID);
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