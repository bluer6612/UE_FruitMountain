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

    // 5. 각도에 따른 거리 계산 수정 - 완전히 부드러운 곡선으로 변경
    // 물리학적으로 정확한 각도-거리 관계 구현 (sin(2θ) 기반 수식)
    float RadAngle = FMath::DegreesToRadians(UseAngle);
    float TwoRadAngle = 2.0f * RadAngle;

    // 최대 투사 거리는 45도에서 발생하는 sin(2θ) 곡선 활용
    // 실제 물리학에서 최대 사거리는 sin(2θ)에 비례하며 이는 45도에서 최대
    float SinTwoTheta = FMath::Sin(TwoRadAngle);

    // 공기 저항 효과를 시뮬레이션하는 추가 조정 (극단 각도에서 더 감소)
    // 저각도와 고각도에서 실제보다 더 빨리 감소하는 공기 저항 효과 모델링
    float AirResistanceEffect = FMath::Pow(FMath::Sin(RadAngle) * FMath::Cos(RadAngle), 0.7f);

    // 최종 거리 비율 계산 (0.4 ~ 1.0 범위로 매핑)
    float DistanceRatio = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 1.0f), FVector2D(0.4f, 1.0f), SinTwoTheta * AirResistanceEffect);

    // 거리 감소가 자연스러운 곡선으로 이루어지도록 추가 스무딩
    // 사인 곡선 기반 스무딩으로 부드러운 변화 보장
    float NormalizedAngle = (UseAngle - MinThrowAngle) / (MaxThrowAngle - MinThrowAngle);
    float SmoothingFactor = 0.9f + 0.1f * FMath::Sin(NormalizedAngle * PI);
    DistanceRatio *= SmoothingFactor;

    // 포물선 궤적의 타겟 위치 계산 - 각도에 따라 조정된 거리 사용
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;

    // 6. 발사 방향 계산
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    Result.LaunchDirection = FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), HorizontalDir.Y * FMath::Cos(ThrowAngleRad), FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();

    // 7. 초기 속도 계산 보완 - 자연스러운 곡선을 위한 수정
    // 항상 물리적으로 일관된 속도 계산
    float InitialSpeedSquared = (Gravity * AdjustedDistance * AdjustedDistance) / 
                            (2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                            (AdjustedDistance * FMath::Tan(ThrowAngleRad) - HeightDifference));

    // 속도가 유효하지 않으면 물리학적으로 정확한 근사치 사용
    if (InitialSpeedSquared <= 0.0f || FMath::IsNaN(InitialSpeedSquared))
    {
        // 연속적인 속도 곡선을 위한 일관된 대체 공식
        float SinTwoTheta = FMath::Sin(2.0f * ThrowAngleRad);
        SinTwoTheta = FMath::Max(SinTwoTheta, 0.1f); // 안정성 보장
        
        // 속도 계산 (기본 물리 공식 + 부드러운 보정 계수)
        Result.InitialSpeed = FMath::Sqrt((Gravity * AdjustedDistance) / SinTwoTheta);
        
        // 자연스러운 속도 곡선을 위한 부드러운 보정
        float AngleNormalized = (UseAngle - MinThrowAngle) / (MaxThrowAngle - MinThrowAngle);
        float SpeedCurve = 1.0f - 0.2f * FMath::Pow(2.0f * (AngleNormalized - 0.5f), 2);
        Result.InitialSpeed *= SpeedCurve;
    }
    else
    {
        Result.InitialSpeed = FMath::Sqrt(InitialSpeedSquared);
    }

    // 속도 범위 제한 (게임 균형을 위해)
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

    // 속도 스무딩 - 극단 각도에서도 부드러운 변화
    float AngleParam = (UseAngle - MinThrowAngle) / (MaxThrowAngle - MinThrowAngle);
    float SpeedAdjustment = 0.9f + 0.1f * FMath::Sin(AngleParam * PI);
    Result.InitialSpeed *= SpeedAdjustment;
    
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
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(FVector2D(MinThrowAngle, MaxThrowAngle), FVector2D(0.15f, 0.9f), UseAngle);
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
    
    UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    return Result;
}