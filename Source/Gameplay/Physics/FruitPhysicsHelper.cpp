#include "FruitPhysicsHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

// const 정적 변수 초기화
const float UFruitPhysicsHelper::MinThrowAngle = 10.0f;
const float UFruitPhysicsHelper::MaxThrowAngle = 60.0f;

// CalculateTrajectoryPoints 함수 수정
TArray<FVector> UFruitPhysicsHelper::CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!World)
        return TrajectoryPoints;
    
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(
        World, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    // 가상의 물리 바디 생성 (시각적으로 표시하지 않고 예측용으로만 사용)
    FPredictProjectilePathParams PredictParams;
    PredictParams.StartLocation = StartLocation;
    PredictParams.LaunchVelocity = PhysicsResult.LaunchVelocity;
    PredictParams.bTraceWithCollision = true;
    PredictParams.ProjectileRadius = 5.0f;
    PredictParams.MaxSimTime = 5.0f;
    PredictParams.SimFrequency = 20;
    PredictParams.OverrideGravityZ = -FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ); // 언리얼 엔진의 물리 설정에서 중력값 가져오기
    PredictParams.DrawDebugType = EDrawDebugTrace::None;
    PredictParams.TraceChannel = ECC_WorldDynamic;
    
    FPredictProjectilePathResult PredictResult;
    bool bHit = UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);
    
    // 예측 결과를 궤적 포인트로 변환
    for (const FPredictProjectilePathPointData& PointData : PredictResult.PathData)
    {
        TrajectoryPoints.Add(PointData.Location);
    }
    
    //UE_LOG(LogTemp, Log, TEXT("예측 궤적: %d개 포인트, 충돌=%s"), 
    //    TrajectoryPoints.Num(), bHit ? TEXT("True") : TEXT("False"));
    
    return TrajectoryPoints;
}

// 통합 물리 계산 함수 구현
FThrowPhysicsResult UFruitPhysicsHelper::CalculateThrowPhysics(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass)
{
    // 정적 캐시 변수 (중복 호출 시 안정성 보장)
    static FThrowPhysicsResult CachedResult;
    static FVector LastStartLocation = FVector::ZeroVector;
    static FVector LastTargetLocation = FVector::ZeroVector;
    static float LastThrowAngle = 0.0f;
    static float LastBallMass = 0.0f;
    static float CacheTimeout = 0.0f;
    
    // 현재 시간 가져오기
    float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
    
    // 입력 파라미터가 이전과 거의 같고, 캐시가 너무 오래되지 않았으면 캐시된 결과 반환
    if (CachedResult.bSuccess && 
        (LastStartLocation - StartLocation).Size() < 1.0f &&
        (LastTargetLocation - TargetLocation).Size() < 1.0f &&
        FMath::Abs(LastThrowAngle - ThrowAngle) < 0.1f &&
        FMath::Abs(LastBallMass - BallMass) < 0.1f &&
        (CurrentTime - CacheTimeout) < 0.1f) // 0.1초 이내 캐시만 사용
    {
        return CachedResult;
    }
    
    // 새 결과 계산
    FThrowPhysicsResult Result;
    
    // 1. 각도 범위 가져오기 & 조정
    float UseAngle = FMath::Clamp(ThrowAngle, MinThrowAngle, MaxThrowAngle);
    float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
    
    // 2. 접시 정보 찾기 - 공유된 캐시 사용
    FVector PlateCenter = TargetLocation; // 기본값
    float PlateTopHeight = 20.0f;
    
    // 접시 위치 초기화 - 아직 초기화되지 않았다면
    if (!UFruitThrowHelper::bPlateCached && World)
    {
        UFruitThrowHelper::InitializePlatePosition(World);
    }
    
    // 항상 캐시된 접시 위치 사용
    if (UFruitThrowHelper::bPlateCached)
    {
        PlateCenter = UFruitThrowHelper::CachedPlateCenter;
        
        // 접시 높이 정보 계산 - 이 부분은 유지 (매번 계산해도 큰 오버헤드 없음)
        if (World)
        {
            TArray<AActor*> PlateActors;
            UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
            
            if (PlateActors.Num() > 0)
            {
                FVector PlateOrigin;
                FVector PlateExtent;
                PlateActors[0]->GetActorBounds(false, PlateOrigin, PlateExtent);
                
                PlateTopHeight = PlateOrigin.Z + PlateExtent.Z + 5.0f;
            }
        }
    }
    
    // 3. 방향 벡터 계산 부분 수정
    FVector DirectionToTarget = PlateCenter - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = PlateTopHeight - StartLocation.Z;
    DirectionToTarget.Z = 0.0f;
    DirectionToTarget.Normalize();

    // 4. 중력 값 가져오기 (추가)
    float Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ);

    // 5. 각도에 따른 거리 계산 부분 수정 - 인위적인 조정 제거
    // 새로운 코드 - 물리학적으로 자연스러운 계산
    float BaseInitialSpeed = 250.0f; // 기본 초기 속도 (모든 각도에서 일정)
    float DistanceRatio = 1.0f;

    // 포물선 궤적의 타겟 위치 계산
    float AdjustedDistance = HorizontalDistance;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;

    // 6. 발사 방향 계산
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    Result.LaunchDirection = FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), HorizontalDir.Y * FMath::Cos(ThrowAngleRad), FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();

    // 7. 초기 속도 계산 수정 (자연스러운 포물선 물리)
    // 투사체 도달에 필요한 속도 계산 (표준 포물선 방정식 사용)
    float InitialSpeedSquared = (Gravity * HorizontalDistance * HorizontalDistance) / 
                              (2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                              (HorizontalDistance * FMath::Tan(ThrowAngleRad) - HeightDifference));

    // 속도가 유효하지 않으면 근사치 사용
    if (InitialSpeedSquared <= 0.0f || FMath::IsNaN(InitialSpeedSquared))
    {
        // 물리적으로 더 정확한 대체 공식:
        // v = sqrt((g*x) / sin(2θ))  (평지에서의 투사체 속도 공식)
        
        // 2θ 값 계산 (각도의 2배)
        float TwoTheta = 2.0f * ThrowAngleRad;
        float SinTwoTheta = FMath::Sin(TwoTheta);
        
        // 0에 가까운 값은 오류 방지
        SinTwoTheta = FMath::Max(SinTwoTheta, 0.1f);
        
        // 대체 속도 계산
        Result.InitialSpeed = FMath::Sqrt((Gravity * HorizontalDistance) / SinTwoTheta);
    }
    else
    {
        Result.InitialSpeed = FMath::Sqrt(InitialSpeedSquared);
    }

    // 물리적으로 합리적인 범위로 제한
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

    // 극단적인 각도에서 안정성 확보를 위한 미세 조정
    // 각도가 너무 낮거나 높을 때 약간의 속도 보정 (강한 각도 종속성 제거)
    if (UseAngle < 15.0f || UseAngle > 55.0f)
    {
        float AngleAdjustment = FMath::GetMappedRangeValueClamped(
            FVector2D(10.0f, 15.0f), 
            FVector2D(0.85f, 1.0f),
            FMath::Clamp(UseAngle, 10.0f, 15.0f)
        );
        
        if (UseAngle > 55.0f)
        {
            AngleAdjustment = FMath::GetMappedRangeValueClamped(
                FVector2D(55.0f, 60.0f), 
                FVector2D(1.0f, 0.9f),
                UseAngle
            );
        }
        
        Result.InitialSpeed *= AngleAdjustment;
    }
    
    // 8. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;

    // 9. 적용할 힘 계산 - 충격량 직접 계산
    // 충격량 = 질량 * 속도변화
    Result.AdjustedForce = BallMass * Result.InitialSpeed;

    // 10. 힘 범위 제한 (극단적인 값 방지)
    float MinForce = 1800.0f;
    float MaxForce = 9000.0f;
    // 질량에 따른 힘 조정
    float MassRatio = BallMass / 30.0f; // 30kg을 기준으로 비율 계산
    MinForce *= MassRatio;
    MaxForce *= MassRatio;

    Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, MinForce, MaxForce);
    
    // 11. 궤적 최고점 높이 계산
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle), FVector2D(0.15f, 0.9f), UseAngle);
    Result.PeakHeight = HorizontalDistance * PeakHeightRatio;
    
    // 12. 계산 성공 표시
    Result.bSuccess = true;
    
    // 계산 결과 캐싱
    LastStartLocation = StartLocation;
    LastTargetLocation = TargetLocation;
    LastThrowAngle = ThrowAngle;
    LastBallMass = BallMass;
    CachedResult = Result;
    CacheTimeout = CurrentTime;
    
    //UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
    //    UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    return Result;
}