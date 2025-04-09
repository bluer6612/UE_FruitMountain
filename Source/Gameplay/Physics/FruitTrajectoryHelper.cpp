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
    FVector PlateCenter = Controller->PlateLocation;
    
    // 2. 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 3. 물리 계산 결과 가져오기
    static FThrowPhysicsResult LastPhysicsResult; // 마지막 결과 저장용 정적 변수
    FThrowPhysicsResult PhysicsResult = UFruitPhysicsHelper::CalculateThrowPhysics(
        World, StableStartLocation, PlateCenter, StableAngle, BallMass);
    
    // 중요: 정적 변수를 사용하여 이전 결과와 비교할 때 카메라 각도 무시
    static float LastAngle = 0.0f;
    static FVector LastStartLoc = FVector::ZeroVector;
    
    // 스폰 위치와 각도만 비교 (카메라 회전은 무시)
    bool bSimilarConditions = 
        FMath::Abs(LastAngle - StableAngle) < 0.1f &&
        (LastStartLoc - StableStartLocation).Size() < 1.0f;
    
    // 결과 안정화 - 동일한 조건에서 이전 결과 재사용
    if (LastPhysicsResult.bSuccess && bSimilarConditions)
    {
        PhysicsResult = LastPhysicsResult;
    }
    else
    {
        LastPhysicsResult = PhysicsResult;
        LastAngle = StableAngle;
        LastStartLoc = StableStartLocation;
    }
    
    // 스폰 위치와 카메라 각도 로깅
    float HorizontalDistance = FVector(PlateCenter - StableStartLocation).Size2D();

    UE_LOG(LogTemp, Warning, TEXT("궤적 업데이트: 카메라각도=%.1f°, 스폰위치=%s, 거리=%.1f"),
        Controller->CameraOrbitAngle, 
        *StableStartLocation.ToString(), 
        HorizontalDistance);
    
    // 5. 물리 기반 궤적 계산 (bezier 궤적은 사용하지 않음)
    TArray<FVector> TrajectoryPoints = CalculateTrajectoryPoints(World, StableStartLocation, PlateCenter, StableAngle, BallMass);

    // 6. 궤적 시각화
    DrawTrajectoryPath(World, TrajectoryPoints, TrajectoryID);
}

// 궤적 시각화 함수 수정
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID)
{
    if (!World || Points.Num() < 2)
    {
        return;
    }
    
    // 이전에 그린 모든 궤적 먼저 비우기 (최대한 확실하게)
    FlushPersistentDebugLines(World);
    
    FColor PathColor = FColor(135, 206, 235, 100);
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
    
    // LineBatcher를 사용하여 궤적 그리기
    if (World->PersistentLineBatcher)
    {
        for (int32 i = 0; i < FilteredPoints.Num() - 1; i++)
        {
            // DepthPriority를 SDPG_World로 설정하여 일반 물체와 동일한 깊이 테스트 적용
            // 이 부분은 미리보기 공 뒤에 가려지는 궤적을 그림
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

// 이 함수는 FruitPhysicsHelper에서 이동됨
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!World)
        return TrajectoryPoints;
    
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = UFruitPhysicsHelper::CalculateThrowPhysics(
        World, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    // 가상의 물리 바디 생성 (시각적으로 표시하지 않고 예측용으로만 사용)
    FPredictProjectilePathParams PredictParams;
    PredictParams.StartLocation = StartLocation;
    PredictParams.LaunchVelocity = PhysicsResult.LaunchVelocity;
    PredictParams.bTraceWithCollision = true;
    PredictParams.ProjectileRadius = 5.0f;
    PredictParams.MaxSimTime = 5.0f;
    PredictParams.SimFrequency = 20;
    PredictParams.OverrideGravityZ = -FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ);
    PredictParams.DrawDebugType = EDrawDebugTrace::None;
    
    // 중요: 충돌 채널 설정 - WorldStatic만 검사하고 물체(FruitBall)은 무시
    PredictParams.TraceChannel = ECC_WorldStatic;
    
    // 중요: 과일 공 무시 설정 추가
    PredictParams.ActorsToIgnore.Empty();
    
    // 모든 FruitBall 찾아서 무시 목록에 추가
    TArray<AActor*> FruitBalls;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FruitBalls);
    PredictParams.ActorsToIgnore = FruitBalls;
    
    // 접시는 충돌 테스트에 포함 (접시에 도달하는지 보여주기 위해)
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // 이미 무시 목록에 있으면 제거
        PredictParams.ActorsToIgnore.Remove(PlateActors[0]);
    }
    
    FPredictProjectilePathResult PredictResult;
    bool bHit = UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);
    
    // 예측 결과를 궤적 포인트로 변환
    for (const FPredictProjectilePathPointData& PointData : PredictResult.PathData)
    {
        TrajectoryPoints.Add(PointData.Location);
    }
    
    return TrajectoryPoints;
}