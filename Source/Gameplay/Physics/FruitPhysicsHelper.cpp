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
    // 물리 데이터 초기화 데이터 구조체 생성
    FPhysicsInitData InitData(World, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    // 0. 캐싱 처리 - 성능 최적화 (분리된 함수 사용)
    FThrowPhysicsResult Result;
    if (UFruitPhysicsInitializer::CheckCachedResult(InitData, Result))
    {
        return Result;
    }
    
    // 1~6. 초기 물리 데이터 계산 (별도 클래스 사용)
    FPhysicsBaseResult BaseResult = UFruitPhysicsInitializer::InitializePhysics(InitData);
    
    // 결과 데이터 초기화
    Result.AdjustedTarget = BaseResult.AdjustedTarget;
    Result.LaunchDirection = BaseResult.LaunchDirection;
    
    // 현재 시간 가져오기
    float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
    
    // 7. 속도 계산 (아래 코드는 원래 코드 유지)
    // 7-1. 기본 속도값 설정
    float BaseSpeed = 250.0f;
    
    // 7-4. 질량에 따른 속도 보정 추가
    float MassCompensationFactor = FMath::Sqrt(BallMass / AFruitBall::DensityFactor);
    
    // 7-5. 최종 초기 속도 계산 - 질량 보정 반영
    Result.InitialSpeed = BaseSpeed * MassCompensationFactor;
    
    // 7-6. 디버그 로깅 추가
    UE_LOG(LogTemp, Log, TEXT("질량 보정 계수: %.2f (질량: %.1f)"), MassCompensationFactor, BallMass);
    
    // 7-7. 물리 시뮬레이션 안정성을 위한 범위 제한
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 200.0f, 350.0f);
    
    // 7-8. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("초기 속도: %.1f (각도계수: %.2f, 거리계수: %.2f)"),
        Result.InitialSpeed, 1.0f, 1.0f);
    
    // 7-9. 속도 범위 제한
    Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 150.0f, 350.0f);

    // 8. 보정값 시스템 (제거 또는 비활성화)
    // 보정 맵이 비어 있을 때만 보정 적용을 허용
    if (FruitTypeCorrections.Num() > 0 || AngleCorrections.Num() > 0) {
        // 학습 데이터가 있을 때만 보정 적용
        int32 FruitTypeKey = FMath::RoundToInt(BallMass);
        float AngleKey = BaseResult.UseAngle; // 반올림하지 않고 실제 각도 사용
        
        if (FruitTypeCorrections.Contains(FruitTypeKey)) {
            Result.InitialSpeed *= FruitTypeCorrections[FruitTypeKey];
        }
    }

    // 9. 자연스러운 속도 계산 - 실제 물리 기반
    // 9-1. 각도에 따른 연속적인 함수 사용 (반올림 대신)
    Result.InitialSpeed = (245.0f - 0.9f * BaseResult.UseAngle) * MassCompensationFactor;

    // 9-3. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;

    // 10. 발사 속도 벡터 계산 (중복 제거)
    // 이미 9-3에서 계산했으므로 제거

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
        BaseResult.UseAngle
    );

    // 13-2. 비선형 증가 효과 (지수 함수로 고각도에서 더 급격한 증가)
    float PoweredHeightRatio = FMath::Pow(DirectAngleHeightRatio, 1.5f);

    // 13-3. 최고점 높이 계산
    Result.PeakHeight = BaseResult.HorizontalDistance * PoweredHeightRatio;

    // 13-4. 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("궤적 최고점: %.1f (높이비율: %.2f, 거리: %.1f)"),
        Result.PeakHeight, PoweredHeightRatio, BaseResult.HorizontalDistance);
    
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
    ValidateParams.OverrideGravityZ = -BaseResult.Gravity;
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
        
        float XYDistance = FVector::Dist(
            FVector(EndPoint.X, EndPoint.Y, 0),
            FVector(BaseResult.PlateCenter.X, BaseResult.PlateCenter.Y, 0)
        );
        
        float ZDistance = FMath::Abs(EndPoint.Z - BaseResult.PlateTopHeight);
        
        // 로깅만 수행하고 보정은 하지 않음
        UE_LOG(LogTemp, Warning, TEXT("검증 결과: XY거리=%.1f, Z거리=%.1f (발사속도=%.1f)"), 
               XYDistance, ZDistance, Result.InitialSpeed);
        
        if (XYDistance > 20.0f || ZDistance > 20.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("정확도가 낮은 투척 감지: 각도=%.1f, 거리=%.1f"), 
                   BaseResult.UseAngle, BaseResult.HorizontalDistance);
        }
        
        // 검증 정보를 통계에 저장하여 나중에 참조 가능하게 함
        // 이 코드는 보정에 사용하지 않고 통계 목적으로만 사용
        int32 FruitTypeKey = FMath::RoundToInt(BallMass);
        if (FruitTypeCorrections.Num() < 10 && !FruitTypeCorrections.Contains(FruitTypeKey))
        {
            // 특정 과일 타입에 대한 첫 10개 유형만 기록 (자동 학습용)
            float AccuracyFactor = 1.0f;
            if (XYDistance > 0.1f) {
                // 정확도 기반 보정 계수 (실제로 보정에 사용하지는 않음)
                AccuracyFactor = 20.0f / FMath::Max(XYDistance, 1.0f);
                AccuracyFactor = FMath::Clamp(AccuracyFactor, 0.8f, 1.2f);
            }
            
            // 로깅만 수행
            UE_LOG(LogTemp, Warning, TEXT("과일 크기 %d의 정확도 계수: %.2f (기록용)"), 
                  FruitTypeKey, AccuracyFactor);
        }
    }
    
    // 16. 결과 마무리 및 반환
    // 16-1. 계산 성공 표시
    Result.bSuccess = true;
    
    // 16-2. 계산 결과 캐싱 - InitializePhysics 클래스의 함수 사용
    UFruitPhysicsInitializer::UpdateCachedResult(InitData, Result, CurrentTime);
    
    // 16-3. 최종 로깅
    UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        BaseResult.UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    // 16-4. 결과 반환
    return Result;
}