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

    // 5. 각도에 따른 거리 계산 수정 - 아치형 곡선으로 조정

    // 최적 각도를 MinAngle과 MaxAngle의 평균으로 설정
    float OptimalAngle = (MinThrowAngle + MaxThrowAngle) / 2.0f; // 35도
    float AngleRange = (MaxThrowAngle - MinThrowAngle); // 50도 범위

    // 현재 각도가 최적 각도에서 얼마나 떨어져 있는지 계산 (0~1 범위)
    float AngleDeviation = FMath::Abs(UseAngle - OptimalAngle) / (AngleRange / 2.0f);

    // U자형 거리 곡선 적용 - 중간 각도에서 멀리, 낮거나 높은 각도에서 가깝게
    // 최적 각도(35도)에서는 0.8, 극단 각도(10도, 60도)에서는 0.2 거리 비율
    float DistanceRatio = FMath::Clamp(0.8f - (AngleDeviation * AngleDeviation * 0.6f), 0.2f, 0.8f);

    // 낮은 각도(10-20도)에서 추가 거리 감소 적용 (낮은 각도 문제 해결)
    if (UseAngle < 20.0f)
    {
        // 10-20도 구간에서 추가 거리 감소
        float LowAngleFactor = FMath::GetMappedRangeValueClamped(
            FVector2D(MinThrowAngle, 20.0f),
            FVector2D(0.4f, 1.0f),  // 최소 40% 수준으로 조정
            UseAngle
        );
        DistanceRatio *= LowAngleFactor;
        
        // 매우 낮은 각도(10-15도)에서는 더 가깝게 - 중앙에 가깝게
        if (UseAngle <= 15.0f)
        {
            float MinDistRatio = FMath::GetMappedRangeValueClamped(
                FVector2D(10.0f, 15.0f),
                FVector2D(0.2f, 0.3f),  // 10도에서는 20% 거리로 강제
                UseAngle
            );
            DistanceRatio = FMath::Min(DistanceRatio, MinDistRatio);
        }
    }

    // 높은 각도(50-60도)에서도 마찬가지로 거리 감소 강화
    if (UseAngle > 50.0f)
    {
        float HighAngleFactor = FMath::GetMappedRangeValueClamped(
            FVector2D(50.0f, MaxThrowAngle),
            FVector2D(1.0f, 0.4f),  // 60도에서는 40% 수준으로 강제 감소
            UseAngle
        );
        DistanceRatio *= HighAngleFactor;
    }

    // 포물선 궤적의 타겟 위치 계산 - 접시 중앙 방향으로 조정
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;

    // 6. 발사 방향 계산 수정 - 각도에 따른 Z 성분 강화
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();

    // 각도에 따라 수직 성분 증가율 대폭 강화 - 단순히 각도가 높을수록 더 큰 HeightBoostFactor
    float AngleNormalized = (UseAngle - MinThrowAngle) / AngleRange; // 0~1 범위
    float HeightBoostFactor;

    // 각도에 따른 높이 조정 - 선형 증가에서 지수 증가로 변경
    // 단순히 각도가 높을수록 더 높은 곡선 (최적값 개념 삭제)
    HeightBoostFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle),
        FVector2D(1.0f, 4.0f), // 10도에서 1.0, 60도에서 4.0 (더 큰 범위)
        UseAngle
    );

    // 고각도에서 더 급격하게 증가하도록 지수 함수 적용
    HeightBoostFactor = FMath::Pow(HeightBoostFactor, 1.4f);

    // 수직 성분 증가 - 더 높은 값 허용
    float VerticalComponent = FMath::Sin(ThrowAngleRad) * HeightBoostFactor;
    // 수직 성분 제한값을 0.99로 상향 (거의 제한 없음)
    VerticalComponent = FMath::Min(VerticalComponent, 0.99f);

    Result.LaunchDirection = FVector(
        HorizontalDir.X * FMath::Cos(ThrowAngleRad),
        HorizontalDir.Y * FMath::Cos(ThrowAngleRad),
        VerticalComponent
    ).GetSafeNormal();

    // 7. 초기 속도 계산 보완 - 안정성 강화

    // 안전장치: HeightDifference가 0에 가까우면 작은 값으로 설정 (계산 오류 방지)
    float SafeHeightDifference = HeightDifference;
    if (FMath::Abs(SafeHeightDifference) < 0.1f)
    {
        SafeHeightDifference = 0.1f;
    }

    // 각도별 맞춤형 속도 계산 - 더 단순하고 일관된 방식
    float BaseSpeed = 250.0f; // 기본 속도
    float OptimalAngleFactor = 1.0f - (AngleDeviation * 0.3f); // 최적 각도에서 최대 속도

    // 거리 비율에 따른 속도 조정 (짧은 거리는 낮은 속도)
    float DistanceSpeedFactor = FMath::Lerp(0.8f, 1.1f, FMath::Clamp(DistanceRatio, 0.0f, 1.0f));

    // 최종 초기 속도 계산
    Result.InitialSpeed = BaseSpeed * OptimalAngleFactor * DistanceSpeedFactor;

    // 속도 범위 제한 (게임 균형을 위해)
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

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

    // 11. 궤적 최고점 높이 계산 - 각도에 따라 비선형으로 증가 (대폭 강화)
    // 각도 정규화 (0~1)
    float NormalizedAngle = (UseAngle - MinThrowAngle) / AngleRange;

    // 각도에 따른 높이 비율 - 각도가 높을수록 급격히 높아지는 단순한 관계로 변경
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle),
        FVector2D(0.1f, 3.5f), // 10도에서 0.1, 60도에서 3.5 (훨씬 더 큰 범위)
        UseAngle
    );

    // 고각도에서 더욱 급격하게 증가하도록 지수 함수 적용
    PeakHeightRatio = FMath::Pow(PeakHeightRatio, 1.5f);

    // 50~60도 구간에서 추가 급증 효과
    if (UseAngle > 50.0f) {
        float HighAngleFactor = FMath::GetMappedRangeValueClamped(
            FVector2D(50.0f, MaxThrowAngle),
            FVector2D(1.0f, 1.5f), // 60도에서 50% 추가 증가
            UseAngle
        );
        PeakHeightRatio *= HighAngleFactor;
    }

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