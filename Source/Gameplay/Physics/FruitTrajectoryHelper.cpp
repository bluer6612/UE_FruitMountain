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
    
    // 기존 궤적 비우기 - 더 확실하게 모든 디버그 선 제거
    FlushPersistentDebugLines(World);
    FlushDebugStrings(World);
    
    // 1. 입력값 안정화 - 입력 좌표를 소수점 아래 1자리까지만 사용
    FVector StableStartLocation = RoundVector(StartLocation, 1);
    FVector StableTargetLocation = RoundVector(TargetLocation, 1);
    float StableAngle = FMath::RoundToFloat(Controller->ThrowAngle * 10.0f) / 10.0f; // 소수점 첫째자리까지
    
    // 2. 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 3. 물리 계산 결과 가져오기
    static FThrowPhysicsResult LastPhysicsResult; // 마지막 결과 저장용 정적 변수
    FThrowPhysicsResult PhysicsResult = UFruitPhysicsHelper::CalculateThrowPhysics(
        World, StableStartLocation, StableTargetLocation, StableAngle, BallMass);
    
    // 4. 결과 안정화 - 너무 작은 변화는 무시
    if (LastPhysicsResult.bSuccess &&
        (PhysicsResult.AdjustedTarget - LastPhysicsResult.AdjustedTarget).Size() < 5.0f &&
        FMath::Abs(PhysicsResult.InitialSpeed - LastPhysicsResult.InitialSpeed) < 5.0f)
    {
        // 작은 변화는 이전 계산 결과 재사용
        PhysicsResult = LastPhysicsResult;
    }
    else
    {
        // 큰 변화가 있을 때만 결과 업데이트
        LastPhysicsResult = PhysicsResult;
    }
    
    // 5. 궤적 계산
    TArray<FVector> TrajectoryPoints;
    
    bool useBezier = false;
    if (useBezier)
    {
        // 베지어 곡선 방식
        TrajectoryPoints = UFruitPhysicsHelper::CalculateBezierPoints(
            StableStartLocation, PhysicsResult.AdjustedTarget, PhysicsResult.PeakHeight, 19);
    }
    else
    {
        // 물리 기반 궤적 (FruitPhysicsHelper로 직접 호출)
        TrajectoryPoints = UFruitPhysicsHelper::CalculateTrajectoryPoints(
            World, StableStartLocation, StableTargetLocation, StableAngle, BallMass);
    }
    
    // 6. 궤적 시각화
    DrawTrajectoryPath(World, TrajectoryPoints, TrajectoryID);
}

// 궤적 시각화 - 통합된 버전
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID)
{
    if (!World || Points.Num() < 2)
    {
        return;
    }
    
    // 이전에 그린 모든 궤적 먼저 비우기 (최대한 확실하게)
    FlushPersistentDebugLines(World);
    
    FColor PathColor = FColor(135, 206, 235, 180);
    int32 MarkerCount = 3;
    float LineThickness = 1.2f;
    
    // 포인트 간 거리가 최소값 이상일 때만 그리기 (너무 조밀한 점은 건너뜀)
    float MinDistance = 5.0f;
    TArray<FVector> FilteredPoints;
    FilteredPoints.Add(Points[0]);
    
    for (int32 i = 1; i < Points.Num(); i++)
    {
        float DistToLast = FVector::Dist(Points[i], FilteredPoints.Last());
        if (DistToLast >= MinDistance || i == Points.Num() - 1) // 첫점과 끝점은 항상 포함
        {
            FilteredPoints.Add(Points[i]);
        }
    }
    
    // 필터링된 포인트로 선 그리기
    for (int32 i = 0; i < FilteredPoints.Num() - 1; i++)
    {
        DrawDebugLine(World, FilteredPoints[i], FilteredPoints[i + 1], 
            PathColor, true, -1.0f, TrajectoryID, LineThickness);
    }
    
    // 주요 지점에만 마커 표시
    if (FilteredPoints.Num() >= MarkerCount)
    {
        for (int32 i = 0; i < MarkerCount; i++)
        {
            int32 Index = (i * (FilteredPoints.Num() - 1)) / (MarkerCount - 1);
            DrawDebugBox(World, FilteredPoints[Index], FVector(0.01f), 
                FQuat::Identity, PathColor, true, -1.0f, TrajectoryID);
        }
    }
}

// 벡터 좌표 반올림 헬퍼 함수 추가
FVector UFruitTrajectoryHelper::RoundVector(const FVector& InVector, int32 DecimalPlaces)
{
    float Multiplier = FMath::Pow(10.0f, (float)DecimalPlaces);
    return FVector(
        FMath::RoundToFloat(InVector.X * Multiplier) / Multiplier,
        FMath::RoundToFloat(InVector.Y * Multiplier) / Multiplier,
        FMath::RoundToFloat(InVector.Z * Multiplier) / Multiplier
    );
}