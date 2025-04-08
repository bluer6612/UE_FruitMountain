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

// 기본 물리 계산 헬퍼 함수 (내부용)
bool UFruitPhysicsHelper::CalculateInitialSpeed(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float& OutInitialSpeed)
{
    // 1. 기본 파라미터 설정
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    float Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ); // 언리얼 엔진의 물리 설정에서 중력값 가져오기
    
    // 2. 수평 거리와 높이 차이 계산
    FVector HorizontalDelta = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).Size();
    float HeightDifference = TargetLocation.Z - StartLocation.Z;
    
    // 3. 초기 속도 계산 (단순 물리 공식 사용)
    float Numerator = Gravity * HorizontalDistance * HorizontalDistance;
    float Denominator = 2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                       (HorizontalDistance * FMath::Tan(ThrowAngleRad) - HeightDifference);
    
    // 분모가 0이 되지 않도록 보호
    if (FMath::Abs(Denominator) < 0.001f)
    {
        Denominator = 0.001f;
    }
    
    float InitialSpeedSquared = Numerator / Denominator;
    
    // 속도가 음수가 나오면 포물선이 도달할 수 없는 것이므로 실패 반환
    if (InitialSpeedSquared <= 0.0f)
    {
        return false;
    }
    
    OutInitialSpeed = FMath::Sqrt(InitialSpeedSquared);
    
    // 안전 범위 제한
    OutInitialSpeed = FMath::Clamp(OutInitialSpeed, 100.0f, 500.0f);
    
    // 각도에 따른 초기 속도 조정 (각도가 높을수록 속도 감소)
    if (ThrowAngle > 45.0f)
    {
        float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), ThrowAngle);
        OutInitialSpeed *= AngleRatio;
    }
    
    return true;
}

// 발사 방향 계산 함수 분리 (내부용)
FVector UFruitPhysicsHelper::CalculateLaunchDirection(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngleRad)
{
    FVector DirectionToTarget = TargetLocation - StartLocation;
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    
    // ThrowAngleRadians를 ThrowAngleRad로 수정
    return FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), HorizontalDir.Y * FMath::Cos(ThrowAngleRad), FMath::Sin(ThrowAngleRad)).GetSafeNormal();
}

// 던지기 속도 계산 함수 - 통합 코드 사용
bool UFruitPhysicsHelper::CalculateThrowVelocity(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass, FVector& OutLaunchVelocity)
{
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(nullptr, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    if (PhysicsResult.bSuccess)
    {
        OutLaunchVelocity = PhysicsResult.LaunchVelocity;
        return true;
    }
    
    // 실패 시 기본값
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    FVector LaunchDirection = CalculateLaunchDirection(StartLocation, TargetLocation, ThrowAngleRad);
    OutLaunchVelocity = LaunchDirection * 200.0f;
    
    return false;
}

// 던지기 파라미터 계산 함수 - 단일 오류 수정
void UFruitPhysicsHelper::CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass)
{
    if (!Controller) return;
    
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, MinThrowAngle, MaxThrowAngle);
    
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(
        Controller->GetWorld(), StartLocation, TargetLocation, UseAngle, BallMass);
    
    // 결과 반환
    OutLaunchDirection = PhysicsResult.LaunchDirection;
    OutAdjustedForce = PhysicsResult.AdjustedForce;
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 파라미터: 각도=%.1f, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, PhysicsResult.InitialSpeed, OutAdjustedForce, BallMass);
}

// 물리 헬퍼 클래스에 새 함수 구현 추가
FVector UFruitPhysicsHelper::CalculateAdjustedTargetLocation(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, FVector& OutPlateCenter, float& OutPlateTopHeight)
{
    // 기본 물리 계산 결과 가져오기
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(World, StartLocation, TargetLocation, ThrowAngle, 30.0f);
    
    // 접시 정보 찾기 (플레이트 액터 검색)
    if (World)
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
        
        if (PlateActors.Num() > 0)
        {
            AActor* PlateActor = PlateActors[0];
            FVector PlateOrigin;
            FVector PlateExtent;
            PlateActor->GetActorBounds(false, PlateOrigin, PlateExtent);
            
            OutPlateCenter = PlateOrigin;
            OutPlateTopHeight = PlateOrigin.Z + PlateExtent.Z + 5.0f;
        }
        else
        {
            OutPlateCenter = TargetLocation;
            OutPlateTopHeight = TargetLocation.Z;
        }
    }
    else
    {
        OutPlateCenter = TargetLocation;
        OutPlateTopHeight = TargetLocation.Z;
    }
    
    return PhysicsResult.AdjustedTarget;
}

float UFruitPhysicsHelper::CalculateTrajectoryPeakHeight(float HorizontalDistance, float ThrowAngle)
{
    // 각도에 따른 높이 비율 계산 (각도가 높을수록 더 높게)
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinThrowAngle, MaxThrowAngle), FVector2D(0.15f, 0.9f), ThrowAngle
    );
    
    // 정점 높이 = 수평 거리 * 높이 비율
    return HorizontalDistance * PeakHeightRatio;
}

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
    
    UE_LOG(LogTemp, Log, TEXT("예측 궤적: %d개 포인트, 충돌=%s"), 
        TrajectoryPoints.Num(), bHit ? TEXT("True") : TEXT("False"));
    
    return TrajectoryPoints;
}

// 통합 물리 계산 함수 구현
FThrowPhysicsResult UFruitPhysicsHelper::CalculateThrowPhysics(
    UWorld* World, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float ThrowAngle, 
    float BallMass)
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
    
    // 3. 방향 벡터 계산
    FVector DirectionToTarget = PlateCenter - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = PlateTopHeight - StartLocation.Z;
    DirectionToTarget.Z = 0.0f;
    DirectionToTarget.Normalize();
    
    // 4. 각도에 따른 비율 계산
    float OptimalAngle = (MinThrowAngle + MaxThrowAngle) * 0.5f;
    float AngleDeviation = FMath::Abs(UseAngle - OptimalAngle) / ((MaxThrowAngle - MinThrowAngle) * 0.5f);
    float DistanceRatio = FMath::Clamp(1.0f - AngleDeviation * 0.7f, 0.3f, 1.0f);
    
    // 5. 조정된 타겟 위치 계산
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;
    
    // 6. 발사 방향 계산
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    Result.LaunchDirection = FVector(
        HorizontalDir.X * FMath::Cos(ThrowAngleRad),
        HorizontalDir.Y * FMath::Cos(ThrowAngleRad),
        FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();
    
    // 7. 초기 속도 계산 (포물선 공식)
    float Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ); // 언리얼 엔진의 물리 설정에서 중력값 가져오기
    FVector Gravity3D = FVector(0, 0, -Gravity); // FThrowPhysicsResult 계산 시 동일한 중력값 사용
    float AdjustedHorizontalDistance = AdjustedDistance;
    float AdjustedHeightDifference = Result.AdjustedTarget.Z - StartLocation.Z;
    
    // 포물선 방정식 사용 - v²cos²(θ) = g*x²/(2*(x*tan(θ) - h))
    float InitialSpeedSquared = (Gravity * AdjustedHorizontalDistance * AdjustedHorizontalDistance) / 
                               (2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                               (AdjustedHorizontalDistance * FMath::Tan(ThrowAngleRad) - AdjustedHeightDifference));
    
    // 음수 체크 및 안전장치
    if (InitialSpeedSquared <= 0.0f)
    {
        // 각도에 따른 기본 속도 사용
        if (UseAngle <= 45.0f)
        {
            Result.InitialSpeed = 200.0f;
        }
        else
        {
            float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), UseAngle);
            Result.InitialSpeed = 200.0f * AngleRatio;
        }
    }
    else
    {
        Result.InitialSpeed = FMath::Sqrt(InitialSpeedSquared);
        Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 100.0f, 500.0f);
        
        // 각도에 따른 초기 속도 조정
        if (UseAngle > 45.0f)
        {
            float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), UseAngle);
            Result.InitialSpeed *= AngleRatio;
        }
    }
    
    // 8. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;
    
    // 9. 적용할 힘 계산 (질량 고려) - 힘 조정 계수 증가
    float ForceAdjustment = 1.5f; // 0.6f에서 1.5f로 증가
    Result.AdjustedForce = BallMass * Result.InitialSpeed * ForceAdjustment;
    
    // 10. 힘 범위 제한 - 힘 범위 확장
    float MinForce = 2000.0f; // 1500.0f에서 2000.0f로 증가
    float MaxForce = 10000.0f; // 7500.0f에서 10000.0f로 증가
    float AdjustedMinForce = MinForce * (BallMass / 30.0f);
    float AdjustedMaxForce = MaxForce * (BallMass / 30.0f);
    
    Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, AdjustedMinForce, AdjustedMaxForce);
    
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
    
    UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    return Result;
}

// 주요 방정식 단순화 - 동일한 공식으로 속도와 궤적 계산
FVector UFruitPhysicsHelper::CalculateLaunchVelocity(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle)
{
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    
    // 중력 가져오기 (언리얼 물리와 동일한 값 사용)
    float Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ);
    
    // 수평 거리 계산
    FVector Delta = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(Delta.X, Delta.Y, 0.0f).Size();
    float HeightDifference = TargetLocation.Z - StartLocation.Z;
    
    // 발사 방향 단위 벡터
    FVector HorizontalDir = FVector(Delta.X, Delta.Y, 0.0f).GetSafeNormal();
    FVector LaunchDirection = FVector(
        HorizontalDir.X * FMath::Cos(ThrowAngleRad),
        HorizontalDir.Y * FMath::Cos(ThrowAngleRad),
        FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();
    
    // 속도 크기 계산 (포물선 공식)
    float SpeedSquared = (Gravity * HorizontalDistance * HorizontalDistance) /
                        (2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) *
                        (HorizontalDistance * FMath::Tan(ThrowAngleRad) - HeightDifference));
    
    float Speed = FMath::Sqrt(FMath::Max(100.0f, SpeedSquared));
    
    // 속도 벡터 반환
    return LaunchDirection * Speed;
}