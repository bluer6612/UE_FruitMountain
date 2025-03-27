#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitThrowHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitThrowHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 공통 볼 생성 함수 - 미리보기 및 실제 볼 생성에 사용
    UFUNCTION(BlueprintCallable, Category="FruitThrow")
    static class AActor* SpawnBall(class AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics = false);
    
    // 공 던지기 기능
    UFUNCTION(BlueprintCallable, Category="FruitThrow")
    static void ThrowFruit(class AFruitPlayerController* Controller);
};