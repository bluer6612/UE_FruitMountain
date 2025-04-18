#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitThrowHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitThrowHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 공 던지기 함수
    UFUNCTION(BlueprintCallable, Category="Fruit Throw")
    static void ThrowFruit(class AFruitPlayerController* Controller);
    
    // 미리보기 공 업데이트 함수
    UFUNCTION(BlueprintCallable, Category="Fruit Throw")// 기존 함수 선언 수정
    static void UpdatePreviewBall(class AFruitPlayerController* Controller, bool bUpdateRotation = true);
};