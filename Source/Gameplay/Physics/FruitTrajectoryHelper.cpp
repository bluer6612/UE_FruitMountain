#include "FruitTrajectoryHelper.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h"
#include "Gameplay/Create/FruitSpawnHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Components/LineBatchComponent.h"
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
    //FlushDebugStrings(World);
    
    // 1. 입력값 안정화 - 입력 좌표를 소수점 아래 1자리까지만 사용
    FVector StableStartLocation = RoundVector(StartLocation, 1);
    float StableAngle = FMath::RoundToFloat(Controller->ThrowAngle * 10.0f) / 10.0f; // 소수점 첫째자리까지
    
    // 접시 위치 초기화 - 아직 초기화되지 않았다면
    if (!UFruitThrowHelper::bPlateCached)
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 위치 미초기화"));
    }
    
    // 항상 캐시된 접시 위치 사용 (더 이상 직접 찾지 않음)
    FVector PlateCenter = UFruitThrowHelper::CachedPlateCenter;
    
    // 안정화된 타겟 위치를 접시 중심으로 설정
    FVector StableTargetLocation = PlateCenter;
    
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

    // 물리 기반 궤적 (FruitPhysicsHelper로 직접 호출), (bezier 궤적은 사용하지 않음)
    TrajectoryPoints = UFruitPhysicsHelper::CalculateTrajectoryPoints(World, StableStartLocation, StableTargetLocation, StableAngle, BallMass);
    
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
    float LineThickness = 0.8f;
    
    // 포인트 간 거리가 최소값 이상일 때만 그리기 (너무 조밀한 점은 건너뜀)
    float MinDistance = 2.5f;
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
    
    // LineBatcher를 사용하여 깊이 테스트를 활성화한 선 그리기
    if (World->PersistentLineBatcher)
    {
        for (int32 i = 0; i < FilteredPoints.Num() - 1; i++)
        {
            // DepthPriority를 SDPG_World로 설정하여 일반 물체와 동일한 깊이 테스트 적용
            World->PersistentLineBatcher->DrawLine(
                FilteredPoints[i], 
                FilteredPoints[i + 1], 
                PathColor, 
                SDPG_World, // 깊이 테스트 활성화 (물체에 가려짐)
                LineThickness, 
                -1.0f // 영구적
            );
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