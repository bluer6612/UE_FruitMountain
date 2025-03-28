#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitPhysicsHelper.generated.h"

/**
 * 과일 게임의 물리 시뮬레이션을 위한 공통 함수 제공 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitPhysicsHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 던지는 힘 계산을 위한 공통 함수
    UFUNCTION(BlueprintCallable, Category="FruitPhysics")
    static void CalculateThrowParameters(
        const FVector& StartPosition,
        const FVector& TargetPosition,
        float BaseThrowForce,
        float& OutAdjustedForce,
        FVector& OutLaunchDirection);
};