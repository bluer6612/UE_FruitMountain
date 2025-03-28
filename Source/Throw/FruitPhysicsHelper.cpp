#include "FruitPhysicsHelper.h"

// 던지는 힘과 방향을 계산하는 공통 함수
void UFruitPhysicsHelper::CalculateThrowParameters(
    const FVector& StartPosition,
    const FVector& TargetPosition,
    float BaseThrowForce,
    float& OutAdjustedForce,
    FVector& OutLaunchDirection)
{
    // 목표까지의 벡터 계산
    FVector ToTargetVector = TargetPosition - StartPosition;
    
    // 수평 거리 계산
    FVector HorizontalDist = ToTargetVector;
    HorizontalDist.Z = 0;
    float HorizontalDistance = HorizontalDist.Size();
    
    // 높이 계수 설정
    float HeightFactor;
    if (HorizontalDistance < 200.0f)
        HeightFactor = 1.2f;
    else if (HorizontalDistance < 400.0f)
        HeightFactor = 1.0f;
    else if (HorizontalDistance < 600.0f)
        HeightFactor = 0.8f;
    else
        HeightFactor = 0.6f;
    
    // 방향 계산
    FVector HorizontalDir = HorizontalDist.GetSafeNormal();
    OutLaunchDirection = HorizontalDir + FVector(0, 0, HeightFactor);
    OutLaunchDirection.Normalize();
    
    // 힘 계수 설정
    float ForceMultiplier;
    if (HorizontalDistance < 200.0f)
        ForceMultiplier = 0.5f;
    else if (HorizontalDistance < 400.0f)
        ForceMultiplier = 0.8f;
    else if (HorizontalDistance < 600.0f)
        ForceMultiplier = 1.0f;
    else
        ForceMultiplier = 1.2f;
    
    // 최종 힘 계산
    OutAdjustedForce = BaseThrowForce * ForceMultiplier;
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 파라미터: 거리=%f, 방향=%s, 힘=%f"),
        HorizontalDistance, *OutLaunchDirection.ToString(), OutAdjustedForce);
}