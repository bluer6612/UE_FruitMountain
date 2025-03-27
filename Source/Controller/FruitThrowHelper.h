#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitThrowHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitThrowHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ThrowFruit 함수를 헬퍼로 분리
    UFUNCTION(BlueprintCallable, Category="FruitThrow")
    static void ThrowFruit(class AFruitPlayerController* Controller);
};