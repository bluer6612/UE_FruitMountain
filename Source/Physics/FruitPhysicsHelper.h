#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitPhysicsHelper.generated.h"

class AFruitPlayerController;

UCLASS()
class UFruitPhysicsHelper : public UObject
{
    GENERATED_BODY()

public:
    // const 정적 변수 (UPROPERTY 없이)
    static const float MinThrowAngle;
    static const float MaxThrowAngle;
    
    // 각도 제한 확인 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static bool IsAngleInValidRange(float Angle);
    
    // 각도 범위 가져오기
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static void GetThrowAngleRange(float& OutMinAngle, float& OutMaxAngle);
    
    // 속도 벡터 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static bool CalculateThrowVelocity(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass, FVector& OutLaunchVelocity);
    
    // 던지기 파라미터 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static void CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass);

private:
    // 내부 헬퍼 함수 - 초기 속도 계산
    static bool CalculateInitialSpeed(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float& OutInitialSpeed);
    
    // 내부 헬퍼 함수 - 발사 방향 계산
    static FVector CalculateLaunchDirection(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngleRad);
};