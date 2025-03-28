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
    // 던지는 힘 계산을 위한 공통 함수 - Controller에서 직접 ThrowForce 사용
    UFUNCTION(BlueprintCallable, Category="FruitPhysics")
    static void CalculateThrowParameters(
        class AFruitPlayerController* Controller,
        const FVector& StartPosition,
        const FVector& TargetPosition,
        float& OutAdjustedForce,
        FVector& OutLaunchDirection);
};