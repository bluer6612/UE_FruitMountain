#include "FruitPhysicsHelper.h"
#include "FruitPhysicsInitializer.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Actors/FruitBall.h"

// const 정적 변수 초기화
const float UFruitPhysicsHelper::MinThrowAngle = 5.f;
const float UFruitPhysicsHelper::MaxThrowAngle = 60.0f;

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

    // 9. 초기 속도 조정 - 기본값 낮추기 (290.0f → 250.0f)
    Result.InitialSpeed = (250.0f - 0.5f * BaseResult.UseAngle) * MassCompensationFactor;
    // 290.0f -> 250.0f로 변경 (15% 감소)

    // 9-3. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;

    // 10. 발사 속도 벡터 계산 (중복 제거)
    // 이미 9-3에서 계산했으므로 제거

    // 11. 힘 계수 조정 - 과도한 힘 줄이기
    Result.AdjustedForce = BallMass * Result.InitialSpeed * 1.35f; 
    // 1.7f -> 1.35f로 변경 (20% 감소)

    // 12. 힘 범위 조정 - 최소/최대 힘 감소
    float MinForce = 2400.0f; // 2800.0f -> 2400.0f
    float MaxForce = 9000.0f; // 12000.0f -> 9000.0f

    // 12-2. 질량 비율 적용
    float MassRatio = BallMass / AFruitBall::DensityFactor;
    MinForce *= MassRatio;
    MaxForce *= MassRatio;

    // 12-3. 힘 범위 제한 적용
    Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, MinForce, MaxForce);

    // 13. 궤적 최고점 높이 계산
    // 13-1. 각도에 비례하는 직접적인 높이 계수 - 20% 증가
    float DirectAngleHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle),
        FVector2D(0.06f, 1.2f), // 0.05f->0.06f, 1.0f->1.2f (20% 증가)
        BaseResult.UseAngle
    );

    // 13-2. 비선형 증가 효과 (지수 함수로 고각도에서 더 급격한 증가)
    // 수정: 지수 계수 증가
    float PoweredHeightRatio = FMath::Pow(DirectAngleHeightRatio, 1.8f); // 1.5f -> 1.8f

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
        
        // 로깅만 수행 - 보정 없음
        UE_LOG(LogTemp, Warning, TEXT("검증 결과: XY거리=%.1f, Z거리=%.1f (발사속도=%.1f)"), 
               XYDistance, ZDistance, Result.InitialSpeed);
        
        // 거리 보정을 완전히 제거하고 일관된 궤적 사용
        // if (XYDistance > 25.0f) { ... } 코드 블록 제거
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