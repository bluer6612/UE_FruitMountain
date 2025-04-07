#include "FruitPhysicsHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

// const 정적 변수 초기화
const float UFruitPhysicsHelper::MinThrowAngle = 5.0f;
const float UFruitPhysicsHelper::MaxThrowAngle = 60.0f;

// 각도 범위 가져오기
void UFruitPhysicsHelper::GetThrowAngleRange(float& OutMinAngle, float& OutMaxAngle)
{
    OutMinAngle = MinThrowAngle;
    OutMaxAngle = MaxThrowAngle;
}

// 각도 제한 확인 함수
bool UFruitPhysicsHelper::IsAngleInValidRange(float Angle)
{
    return (Angle >= MinThrowAngle && Angle <= MaxThrowAngle);
}

// 기본 물리 계산 헬퍼 함수 (내부용)
bool UFruitPhysicsHelper::CalculateInitialSpeed(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float& OutInitialSpeed)
{
    // 1. 기본 파라미터 설정
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    float Gravity = 980.0f; // 중력 가속도 (양수로 설정)
    
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
    // 초기 속도 계산
    float InitialSpeed;
    bool bSuccess = CalculateInitialSpeed(StartLocation, TargetLocation, ThrowAngle, InitialSpeed);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("초기 속도 계산 실패: 타겟=%s"), *TargetLocation.ToString());
        return false;
    }

    // 발사 방향 계산
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    FVector LaunchDirection = CalculateLaunchDirection(StartLocation, TargetLocation, ThrowAngleRad);

    // 초기 속도 벡터 계산
    OutLaunchVelocity = LaunchDirection * InitialSpeed;

    UE_LOG(LogTemp, Log, TEXT("발사 속도 계산: 방향=%s, 속도=%.2f"), *LaunchDirection.ToString(), InitialSpeed);

    return true;
}

// 던지기 파라미터 계산 함수 - 단일 오류 수정
void UFruitPhysicsHelper::CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass)
{
    // 목표 지점까지의 벡터 계산
    FVector DirectionToTarget = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = DirectionToTarget.Z;
    
    // 던지기 각도 설정
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, MinThrowAngle, MaxThrowAngle);
    float ThrowAngleRadians = FMath::DegreesToRadians(UseAngle);
    
    // 삼각함수 계산
    float CosTheta = FMath::Cos(ThrowAngleRadians);
    float SinTheta = FMath::Sin(ThrowAngleRadians);
    
    // 1. 원하는 초기 속력 계산 (질량 독립적)
    float InitialSpeed;
    
    // 각도에 따른 속도 조정
    if (UseAngle <= 45.0f)
    {
        InitialSpeed = 300.0f;  // 기준 속도
    }
    else
    {
        // 45도 초과: 각도가 높을수록 속도 감소
        float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), UseAngle);
        InitialSpeed = 300.0f * AngleRatio;
    }
    
    // 2. 방향 벡터 계산 (모든 질량에 동일)
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    OutLaunchDirection = FVector(HorizontalDir.X * CosTheta, HorizontalDir.Y * CosTheta, SinTheta).GetSafeNormal();
    
    // 3. 힘 = 질량 * 속도
    // 동일한 가속도를 얻기 위해 질량에 비례하게 힘을 조정
    OutAdjustedForce = BallMass * InitialSpeed;
    
    // 4. 안전장치: 너무 작거나 큰 힘은 제한
    float MinForce = 3000.0f;
    float MaxForce = 15000.0f;
    
    // 질량에 관계없이 같은 궤적을 유지하기 위해 힘의 범위를 질량에 비례하게 조정
    float AdjustedMinForce = MinForce * (BallMass / 30.0f);  // 30을 기준 질량으로 설정
    float AdjustedMaxForce = MaxForce * (BallMass / 30.0f);
    
    // 안전장치 적용
    if (OutAdjustedForce < AdjustedMinForce)
    {
        OutAdjustedForce = AdjustedMinForce;
    }
    else if (OutAdjustedForce > AdjustedMaxForce)
    {
        OutAdjustedForce = AdjustedMaxForce;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%.1f, 높이차=%.1f, 각도=%.1f도, 초기속도=%.1f, 힘=%.1f, 질량=%.1f"),
        HorizontalDistance, HeightDifference, UseAngle, InitialSpeed, OutAdjustedForce, BallMass);
}

// 물리 헬퍼 클래스에 새 함수 구현 추가
FVector UFruitPhysicsHelper::CalculateAdjustedTargetLocation(
    UWorld* World, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float ThrowAngle, 
    FVector& OutPlateCenter, 
    float& OutPlateTopHeight)
{
    // 접시 중심 찾기
    OutPlateCenter = FVector::ZeroVector;
    OutPlateTopHeight = 0.0f;

    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);

    if (PlateActors.Num() > 0)
    {
        AActor* PlateActor = PlateActors[0];
        OutPlateCenter = PlateActor->GetActorLocation();

        // 접시 상단 높이 계산
        FBox Bounds = PlateActor->GetComponentsBoundingBox();
        OutPlateTopHeight = Bounds.Max.Z;

        // 던지기 각도에 따라 타겟 위치 조정
        float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);

        // 타겟 방향 벡터 계산
        FVector DirectionToTarget = TargetLocation - StartLocation;
        FVector HorizontalDirection = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();

        // 접시 중심점에서 조정된 위치 계산
        FVector AdjustedTarget = OutPlateCenter;

        // 수평 거리와 높이 조정
        float HorizontalDistance = FVector::Dist2D(OutPlateCenter, StartLocation);
        float AdjustedHorizontalDistance = HorizontalDistance * 0.9f; // 거리 계수 조정
        AdjustedTarget = OutPlateCenter - (HorizontalDirection * AdjustedHorizontalDistance);

        // 약간의 높이 마진 추가
        AdjustedTarget.Z = OutPlateTopHeight + 10.0f; // 마진을 10.0f로 증가

        return AdjustedTarget;
    }

    // 접시를 찾지 못한 경우 원래 타겟 사용
    return TargetLocation;
}

float UFruitPhysicsHelper::CalculateTrajectoryPeakHeight(float HorizontalDistance, float ThrowAngle, float MinAngle, float MaxAngle)
{
    // 각도에 따른 높이 비율 계산 (각도가 높을수록 더 높게)
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinAngle, MaxAngle), FVector2D(0.15f, 0.9f), ThrowAngle
    );
    
    // 정점 높이 = 수평 거리 * 높이 비율
    return HorizontalDistance * PeakHeightRatio;
}