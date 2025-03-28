#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지는 힘과 방향을 계산하는 공통 함수 - 질량 독립적으로 수정
void UFruitPhysicsHelper::CalculateThrowParameters(
    AFruitPlayerController* Controller,
    const FVector& StartPosition,
    const FVector& TargetPosition,
    float& OutAdjustedForce,
    FVector& OutLaunchDirection,
    float BallMass)
{
    // Controller가 없으면 기본값 사용
    if (!Controller)
    {
        OutAdjustedForce = 0.0f;
        OutLaunchDirection = FVector::ZeroVector;
        return;
    }

    // Controller에서 ThrowForce 값을 가져옴
    float BaseThrowForce = Controller->ThrowForce;
    
    // 목표까지의 벡터 계산
    FVector ToTargetVector = TargetPosition - StartPosition;
    
    // 수평 거리 계산
    FVector HorizontalDist = ToTargetVector;
    HorizontalDist.Z = 0;
    float HorizontalDistance = HorizontalDist.Size();
    
    // 수직 거리 계산 (높이 차이)
    float VerticalDistance = TargetPosition.Z - StartPosition.Z;
    
    // 중력 가속도 (언리얼 기본값 = 980.0)
    const float Gravity = 980.0f;
    
    // 수평 방향 단위 벡터
    FVector HorizontalDir = HorizontalDist;
    HorizontalDir.Normalize();
    
    // 거리에 따른 높이 계수 조정
    float HeightFactor = 0.8f; // 기본값
    
    if (HorizontalDistance < 200.0f)
        HeightFactor = 0.6f;
    else if (HorizontalDistance < 400.0f)
        HeightFactor = 0.8f;
    else if (HorizontalDistance < 600.0f)
        HeightFactor = 1.0f;
    else
        HeightFactor = 1.2f;
    
    // 방향 벡터 계산
    OutLaunchDirection = HorizontalDir + FVector(0, 0, HeightFactor);
    OutLaunchDirection.Normalize();
    
    // 거리에 따른 힘 계수 조정
    float ForceMultiplier = 1.0f; // 기본값
    
    if (HorizontalDistance < 200.0f)
        ForceMultiplier = 0.6f;
    else if (HorizontalDistance < 400.0f)
        ForceMultiplier = 0.8f;
    else if (HorizontalDistance < 600.0f)
        ForceMultiplier = 1.0f;
    else
        ForceMultiplier = 1.2f;
    
    // 중요: 질량에 관계없이 일정한 속도를 갖도록 힘 계산
    // F = m * a 이므로, 동일한 가속도를 위해 질량에 비례한 힘 적용
    float ScaledForce = BaseThrowForce * ForceMultiplier * BallMass;
    
    // 최종 힘 계산
    OutAdjustedForce = ScaledForce;
    
    // 초기 속도 계산 (F = m * a 이므로 v = F/m)
    float InitialVelocity = ScaledForce / BallMass; // 질량과 무관하게 동일한 속도
    
    // 발사 각도 계산
    float LaunchAngle = FMath::Atan2(OutLaunchDirection.Z, FMath::Sqrt(FMath::Square(OutLaunchDirection.X) + FMath::Square(OutLaunchDirection.Y)));
    
    // 비행 시간 계산 (대략적인 값)
    float FlightTime = HorizontalDistance / (InitialVelocity * FMath::Cos(LaunchAngle));
    
    // 예상 착륙 지점 Z 좌표
    float LandingZ = StartPosition.Z + InitialVelocity * FMath::Sin(LaunchAngle) * FlightTime - 0.5f * Gravity * FMath::Square(FlightTime);
    
    // Z 오차 계산 및 각도 조정
    float ZError = TargetPosition.Z - LandingZ;
    if (FMath::Abs(ZError) > 10.0f)
    {
        // 각도 조정 (오차가 크면 발사 각도 조정)
        HeightFactor += (ZError > 0) ? 0.2f : -0.2f;
        OutLaunchDirection = HorizontalDir + FVector(0, 0, HeightFactor);
        OutLaunchDirection.Normalize();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 파라미터: 거리=%f, 방향=%s, 힘=%f, 질량=%f"),
        HorizontalDistance, *OutLaunchDirection.ToString(), OutAdjustedForce, BallMass);
}