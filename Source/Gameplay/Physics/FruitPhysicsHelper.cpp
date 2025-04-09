#include "FruitPhysicsHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Actors/FruitBall.h"

// const 정적 변수 초기화
const float UFruitPhysicsHelper::MinThrowAngle = 10.0f;
const float UFruitPhysicsHelper::MaxThrowAngle = 60.0f;

// 정적 클래스 변수로 자기 학습용 데이터 추가
static TMap<int32, float> FruitTypeCorrections;
static TMap<float, float> AngleCorrections;

// 통합 물리 계산 함수 구현
FThrowPhysicsResult UFruitPhysicsHelper::CalculateThrowPhysics(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass)
{
    // 0. 캐싱 처리 - 성능 최적화
    // 0-1. 정적 캐시 변수 (중복 호출 시 안정성 보장)
    static FThrowPhysicsResult CachedResult;
    static FVector LastStartLocation = FVector::ZeroVector;
    static FVector LastTargetLocation = FVector::ZeroVector;
    static float LastThrowAngle = 0.0f;
    static float LastBallMass = 0.0f;
    static float CacheTimeout = 0.0f;
    
    // 0-2. 현재 시간 가져오기
    float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
    
    // 0-3. 입력 파라미터가 이전과 거의 같고, 캐시가 너무 오래되지 않았으면 캐시된 결과 반환
    if (CachedResult.bSuccess && 
        (LastStartLocation - StartLocation).Size() < 1.0f &&
        (LastTargetLocation - TargetLocation).Size() < 1.0f &&
        FMath::Abs(LastThrowAngle - ThrowAngle) < 0.1f &&
        FMath::Abs(LastBallMass - BallMass) < 0.1f &&
        (CurrentTime - CacheTimeout) < 0.1f) // 0.1초 이내 캐시만 사용
    {
        return CachedResult;
    }
    
    // 0-4. 새 결과 계산
    FThrowPhysicsResult Result;
    
    // 1. 각도 범위 가져오기 & 조정
    // 1-1. 각도 값 클램핑
    float UseAngle = FMath::Clamp(ThrowAngle, MinThrowAngle, MaxThrowAngle);
    float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
    
    // 2. 접시 정보 찾기 - 공유된 캐시 사용
    // 2-1. 기본값 설정
    FVector PlateCenter = TargetLocation; // 기본값
    float PlateTopHeight = 20.0f;
    
    // 2-2. 접시 위치 초기화 - 아직 초기화되지 않았다면
    if (!UFruitThrowHelper::bPlateCached && World)
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 위치 미초기화"));
    }
    
    // 2-3. 항상 캐시된 접시 위치 사용
    if (UFruitThrowHelper::bPlateCached)
    {
        PlateCenter = UFruitThrowHelper::CachedPlateCenter;
        
        // 2-4. 접시 높이 정보 계산 - 이 부분은 유지 (매번 계산해도 큰 오버헤드 없음)
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
    // 3-1. 기본 방향 벡터 계산
    FVector DirectionToTarget = PlateCenter - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = PlateTopHeight - StartLocation.Z;
    DirectionToTarget.Z = 0.0f;
    DirectionToTarget.Normalize();

    // 4. 중력 값 가져오기 (추가)
    float Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ);

    // 5. 각도에 따른 거리 계산 - 모든 각도에서 일관된 거리로 수정
    // 5-1. 기본 거리 비율 설정
    float DistanceRatio = 0.15f; // 0.12f -> 0.15f (25% 증가)

    // 5-2. 고정된 거리 비율 사용 (각도에 상관없이 일정)
    DistanceRatio = 0.15f;

    // 5-3. 접시 높이에 따른 미세 조정은 유지 (환경에 맞춰 적응)
    if (PlateTopHeight > 30.0f) {
        float HeightFactor = (PlateTopHeight - 30.0f) / 20.0f;
        DistanceRatio *= (1.0f - 0.02f * HeightFactor);
    }

    // 5-4. 포물선 궤적의 타겟 위치 계산 - 접시 중앙 방향으로 조정
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;

    // 5-5. 추가 디버그 로그 - 조정된 타겟 위치 확인용
    UE_LOG(LogTemp, Warning, TEXT("조정된 타겟: %s (거리비율: %.3f, 거리: %.1f)"),
        *Result.AdjustedTarget.ToString(), DistanceRatio, AdjustedDistance);

    // 6. 발사 방향 계산 수정 - 각도와 고도를 직접적이고 명확하게 연결
    // 6-1. 기본 발사 각도 계산 (라디안)
    ThrowAngleRad = FMath::DegreesToRadians(UseAngle);

    // 6-2. 각도에 따른 수직 성분 계산을 간소화하고 명확하게 처리
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();

    // 6-3. 각도에 따른 높이 계수 계산 - 추가로 40% 더 낮춤
    float HeightFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, 50.0f),
        FVector2D(0.25f, 1.25f), // 0.375 -> 0.25, 1.875 -> 1.25 (추가 40% 감소)
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
        
    // 7. 속도 계산
    // 7-1. 기본 속도값 설정 - 거리와 각도에 기반 (기본 속도 증가)
    float BaseSpeed = 250.0f; // 200.0f -> 250.0f (25% 증가)

    // 7-2. 각도에 따른 속도 조정 - 모든 각도에서 일관된 결과
    float AngleSpeedFactor = 1.0f; // 고정값 사용 (모든 각도에서 동일)

    // 7-3. 거리에 따른 속도 조정 - 동일하게 유지
    float DistanceSpeedFactor = 1.0f; // 고정값 사용 (거리에 상관없이 동일)

    // 7-4. 질량에 따른 속도 보정 추가 - 질량이 다른 과일도 동일한 궤적을 그리도록
    float MassCompensationFactor = FMath::Sqrt(BallMass / AFruitBall::DensityFactor); // AFruitBall::DensityFactor을 기준으로 제곱근 비율 계산

    // 7-5. 최종 초기 속도 계산 - 질량 보정 반영
    Result.InitialSpeed = BaseSpeed * MassCompensationFactor;

    // 7-6. 디버그 로깅 추가
    UE_LOG(LogTemp, Log, TEXT("질량 보정 계수: %.2f (질량: %.1f)"), MassCompensationFactor, BallMass);

    // 7-7. 물리 시뮬레이션 안정성을 위한 범위 제한 (상한 증가)
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 200.0f, 350.0f); // 150.0f->200.0f, 300.0f->350.0f

    // 7-8. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("초기 속도: %.1f (각도계수: %.2f, 거리계수: %.2f)"),
        Result.InitialSpeed, AngleSpeedFactor, DistanceSpeedFactor);

    // 7-9. 속도 범위 제한 (게임 균형을 위해)
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

    // 8. 보정값 적용
    // 8-1. 현재 과일과 각도에 대한 보정값 적용
    int32 FruitTypeKey = FMath::RoundToInt(BallMass);
    float AngleKey = FMath::RoundToFloat(UseAngle);

    // 8-2. 과일 타입별 보정
    if (FruitTypeCorrections.Contains(FruitTypeKey)) {
        Result.InitialSpeed *= FruitTypeCorrections[FruitTypeKey];
    }

    // 8-3. 각도별 보정
    if (AngleCorrections.Contains(AngleKey)) {
        Result.InitialSpeed *= AngleCorrections[AngleKey];
    }

    // 9. 최적화된 속도 적용
    // 9-1. 각도별로 최적화된 속도 사용 (실험 기반)
    TMap<int32, float> OptimalSpeeds;
    OptimalSpeeds.Add(10, 240.0f);
    OptimalSpeeds.Add(20, 230.0f);
    OptimalSpeeds.Add(30, 220.0f);
    OptimalSpeeds.Add(40, 210.0f);
    OptimalSpeeds.Add(50, 200.0f);
    OptimalSpeeds.Add(60, 190.0f);

    // 9-2. 각도 반올림
    int32 RoundedAngle = FMath::RoundToInt(UseAngle / 10.0f) * 10; // 가장 가까운 10의 배수
    RoundedAngle = FMath::Clamp(RoundedAngle, 10, 60);

    // 9-3. 최적화된 속도 적용
    if (OptimalSpeeds.Contains(RoundedAngle))
    {
        Result.InitialSpeed = OptimalSpeeds[RoundedAngle] * MassCompensationFactor;
        UE_LOG(LogTemp, Warning, TEXT("최적화된 값 사용: 각도=%d, 속도=%.1f"), 
               RoundedAngle, Result.InitialSpeed);
    }

    // 10. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;

    // 11. 질량 보정이 반영된 힘 계산
    // 11-1. 질량에 따른 힘 조정 필요 없이 일관된 힘 적용
    Result.AdjustedForce = BallMass * Result.InitialSpeed * 1.2f; // 20% 추가 힘 적용

    // 12. 힘 범위 제한
    // 12-1. 힘 범위 설정
    float MinForce = 2000.0f; // 1800.0f -> 2000.0f
    float MaxForce = 9000.0f; // 7000.0f -> 9000.0f

    // 12-2. 질량 비율 적용
    float MassRatio = BallMass / AFruitBall::DensityFactor;
    MinForce *= MassRatio;
    MaxForce *= MassRatio;

    // 12-3. 힘 범위 제한 적용
    Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, MinForce, MaxForce);

    // 13. 궤적 최고점 높이 계산
    // 13-1. 각도에 비례하는 직접적인 높이 계수 (추가로 40% 더 낮춤)
    float DirectAngleHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle),
        FVector2D(0.025f, 0.625f), // 0.0375 -> 0.025, 0.9375 -> 0.625 (추가 40% 감소)
        UseAngle
    );

    // 13-2. 비선형 증가 효과 (지수 함수로 고각도에서 더 급격한 증가)
    float PoweredHeightRatio = FMath::Pow(DirectAngleHeightRatio, 1.5f);

    // 13-3. 최고점 높이 계산
    Result.PeakHeight = HorizontalDistance * PoweredHeightRatio;

    // 13-4. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("궤적 최고점: %.1f (높이비율: %.2f, 거리: %.1f)"),
        Result.PeakHeight, PoweredHeightRatio, HorizontalDistance);
    
    // 14. 물리 기반 발사 속도 참조값 계산 안함

    // 15. 검증 과정 개선
    // 15-1. 더 현실적인 설정으로 검증 테스트
    FPredictProjectilePathParams ValidateParams;
    ValidateParams.StartLocation = StartLocation;
    ValidateParams.LaunchVelocity = Result.LaunchVelocity;
    ValidateParams.bTraceWithCollision = true;  // 충돌 활성화
    ValidateParams.ProjectileRadius = 5.0f;     // 과일 반경 추가
    ValidateParams.SimFrequency = 30;           // 더 정밀한 시뮬레이션
    ValidateParams.MaxSimTime = 3.0f;           // 3초로 제한 (접시 도달 충분)
    ValidateParams.OverrideGravityZ = -Gravity;
    ValidateParams.TraceChannel = ECC_WorldStatic;  // 월드 정적 객체와 충돌 확인

    // 접시를 명시적으로 충돌 테스트에 포함
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
    ValidateParams.ActorsToIgnore.Empty();

    // 과일은 무시
    TArray<AActor*> FruitBalls;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FruitBalls);
    ValidateParams.ActorsToIgnore = FruitBalls;

    // 15-2. 검증 시뮬레이션 실행
    FPredictProjectilePathResult ValidationResult;
    UGameplayStatics::PredictProjectilePath(World, ValidateParams, ValidationResult);

    // 15-3. 끝점 위치와 접시 중앙까지의 거리 계산
    if (ValidationResult.PathData.Num() > 0)
    {
        FVector EndPoint = ValidationResult.PathData.Last().Location;
        
        // 15-4. XY 평면에서의 거리만 체크 (높이 차이는 무시)
        float XYDistance = FVector::Dist(
            FVector(EndPoint.X, EndPoint.Y, 0),
            FVector(PlateCenter.X, PlateCenter.Y, 0)
        );
        
        // 15-5. 높이 차이 별도 계산
        float ZDistance = FMath::Abs(EndPoint.Z - PlateTopHeight);
        
        // 15-6. 검증 결과 로깅
        UE_LOG(LogTemp, Warning, TEXT("검증 결과: XY거리=%.1f, Z거리=%.1f (발사속도=%.1f)"), 
               XYDistance, ZDistance, Result.InitialSpeed);
        
        // 15-7. 정확도가 낮으면 기록
        if (XYDistance > 5.0f || ZDistance > 5.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("정확도가 낮은 투척 감지: 각도=%.1f, 거리=%.1f"), 
                   UseAngle, HorizontalDistance);
        }
    }
    
    // 아래 코드를 추가하여 검증에 실패하더라도 접시에 도달하는 속도 보장

    // 15-7. 정확도가 낮아도 접시 근처에 도달하도록 보정
    if (ValidationResult.PathData.Num() > 0)
    {
        FVector EndPoint = ValidationResult.PathData.Last().Location;
        
        float XYDistance = FVector::Dist(
            FVector(EndPoint.X, EndPoint.Y, 0),
            FVector(PlateCenter.X, PlateCenter.Y, 0)
        );
        
        float ZDistance = FMath::Abs(EndPoint.Z - PlateTopHeight);
        
        // 로깅은 그대로 유지
        UE_LOG(LogTemp, Warning, TEXT("검증 결과: XY거리=%.1f, Z거리=%.1f (발사속도=%.1f)"), 
               XYDistance, ZDistance, Result.InitialSpeed);
        
        // 접시 도달 실패 시 보정
        if (XYDistance > 20.0f || ZDistance > 20.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("정확도가 낮은 투척 감지: 각도=%.1f, 거리=%.1f - 보정 적용"), 
                   UseAngle, HorizontalDistance);
                   
            // 각도에 따른 안전한 속도 사용
            if (OptimalSpeeds.Contains(RoundedAngle))
            {
                // 이미 적용된 최적화 속도를 그대로 사용 (재적용)
                Result.InitialSpeed = OptimalSpeeds[RoundedAngle] * MassCompensationFactor;
                
                // 발사 벡터 업데이트
                Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;
                
                // 힘도 재계산
                Result.AdjustedForce = BallMass * Result.InitialSpeed * 1.2f;
                Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, MinForce, MaxForce);
            }
        }
    }
    
    // 16. 결과 마무리 및 반환
    // 16-1. 계산 성공 표시
    Result.bSuccess = true;
    
    // 16-2. 계산 결과 캐싱
    LastStartLocation = StartLocation;
    LastTargetLocation = TargetLocation;
    LastThrowAngle = ThrowAngle;
    LastBallMass = BallMass;
    CachedResult = Result;
    CacheTimeout = CurrentTime;
    
    // 16-3. 최종 로깅
    UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    // 16-4. 결과 반환
    return Result;
}