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

    // 5. 각도에 따른 거리 계산 - 부드러운 곡선으로 개선
    // U자형 거리 곡선 적용 - 중간 각도에서 멀리, 낮거나 높은 각도에서 가깝게
    // 코사인 함수를 사용하여 부드러운 곡선 생성
    float AngleRange = MaxThrowAngle - MinThrowAngle;
    float NormalizedAngle = (UseAngle - MinThrowAngle) / AngleRange; // 0~1 범위로 정규화
    float CosineValue = FMath::Cos((NormalizedAngle * 2.0f - 1.0f) * PI); // -1~1 코사인 곡선
    float DistanceRatio = FMath::Lerp(0.2f, 0.8f, (CosineValue + 1.0f) * 0.5f); // 0.2~0.8 범위로 매핑

    // 낮은 각도와 높은 각도에서의 추가 조정을 부드러운 함수로 대체
    if (UseAngle < 20.0f) {
        float LowAngleSmooth = FMath::SmoothStep(10.0f, 20.0f, UseAngle);
        DistanceRatio *= FMath::Lerp(0.4f, 1.0f, LowAngleSmooth);
        
        if (UseAngle < 15.0f) {
            float VeryLowAngleSmooth = FMath::SmoothStep(10.0f, 15.0f, UseAngle);
            float MinRatio = FMath::Lerp(0.2f, 0.3f, VeryLowAngleSmooth);
            DistanceRatio = FMath::Min(DistanceRatio, MinRatio);
        }
    }

    // 포물선 궤적의 타겟 위치 계산 - 접시 중앙 방향으로 조정
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;

    // 6. 발사 방향 계산 수정 - 각도와 고도를 직접적이고 명확하게 연결
    // 6-1. 기본 발사 각도 계산 (라디안)
    ThrowAngleRad = FMath::DegreesToRadians(UseAngle);

    // 6-2. 각도에 따른 수직 성분 계산을 간소화하고 명확하게 처리
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();

    // 6-3. 각도에 따른 높이 계수 계산 - 25% 낮춤
    float HeightFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, 50.0f),
        FVector2D(0.375f, 1.875f), // 0.5 -> 0.375, 2.5 -> 1.875 (25% 감소)
        UseAngle
    );

    // 6-4. 수직 성분은 sin(각도)에 비례하고, 높이 계수를 곱해 증폭
    float VerticalMultiplier = FMath::Sin(ThrowAngleRad) * HeightFactor;

    // 6-5. 수평 성분은 cos(각도)에 비례
    float HorizontalMultiplier = FMath::Cos(ThrowAngleRad);

    // 6-6. 최종 발사 방향 계산 - 정규화로 방향만 유지
    Result.LaunchDirection = FVector(
        HorizontalDir.X * HorizontalMultiplier,
        HorizontalDir.Y * HorizontalMultiplier,
        VerticalMultiplier
    ).GetSafeNormal();

    // 6-7. 디버그 로깅 추가
    UE_LOG(LogTemp, Warning, TEXT("발사 각도: %.1f°, 높이계수: %.2f, 수직성분: %.2f, 수평성분: %.2f"),
        UseAngle, HeightFactor, VerticalMultiplier, HorizontalMultiplier);

    // 7. 초기 속도 계산 보완 - 단순화 및 명확화
    // 7-1. 기본 속도값 설정 - 거리와 각도에 기반
    float BaseSpeed = 250.0f;

    // 7-2. 각도에 따른 속도 조정 - 연속적인 곡선 사용
    // 전체 각도 범위에서 부드럽게 변화하는 속도 팩터
    float AngleSpeedFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, 50.0f),
        FVector2D(0.8f, 1.4f),
        UseAngle
    );
    
    // 7-3. 거리에 따른 속도 조정 - 가까운 거리는 더 낮은 속도
    float DistanceSpeedFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 1.0f),
        FVector2D(0.7f, 1.0f),
        DistanceRatio
    );

    // 7-4. 최종 초기 속도 계산
    Result.InitialSpeed = BaseSpeed * AngleSpeedFactor * DistanceSpeedFactor;

    // 7-5. 물리 시뮬레이션 안정성을 위한 범위 제한
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

    // 7-6. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("초기 속도: %.1f (각도계수: %.2f, 거리계수: %.2f)"),
        Result.InitialSpeed, AngleSpeedFactor, DistanceSpeedFactor);

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

    // 11. 궤적 최고점 높이 계산 - 직접적이고 일관된 방식
    // 11-1. 각도에 비례하는 직접적인 높이 계수 (25% 더 낮춤)
    float DirectAngleHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle),
        FVector2D(0.0375f, 0.9375f), // 0.05 -> 0.0375, 1.25 -> 0.9375 (25% 감소)
        UseAngle
    );

    // 11-2. 비선형 증가 효과 (지수 함수로 고각도에서 더 급격한 증가)
    float PoweredHeightRatio = FMath::Pow(DirectAngleHeightRatio, 1.5f);

    // 11-3. 최고점 높이 계산
    Result.PeakHeight = HorizontalDistance * PoweredHeightRatio;

    // 11-4. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("궤적 최고점: %.1f (높이비율: %.2f, 거리: %.1f)"),
        Result.PeakHeight, PoweredHeightRatio, HorizontalDistance);
    
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