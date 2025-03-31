#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitPhysicsHelper.generated.h"

UCLASS()
class UFruitPhysicsHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 던지기 파라미터 계산 함수
    UFUNCTION(BlueprintCallable, Category="Physics")
    static void CalculateThrowParameters(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        float& OutAdjustedForce,
        FVector& OutLaunchDirection,
        float BallMass);

    // const 정적 변수로 선언 (UPROPERTY 없이)
    static const float MinThrowAngle;
    static const float MaxThrowAngle;
    
    // 각도 제한 확인 함수
    UFUNCTION(BlueprintCallable, Category = "Physics Globals")
    static bool IsAngleInValidRange(float Angle);
    
    // 각도 범위 가져오기
    UFUNCTION(BlueprintCallable, Category = "Physics Globals")
    static void GetThrowAngleRange(float& OutMinAngle, float& OutMaxAngle);
};