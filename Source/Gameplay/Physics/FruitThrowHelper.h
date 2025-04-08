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
    UFUNCTION(BlueprintCallable, Category="Fruit Throw")
    static void UpdatePreviewBall(class AFruitPlayerController* Controller);
    
    // 접시 위치 캐싱용 정적 변수 (클래스 간 공유)
    static FVector CachedPlateCenter;
    static bool bPlateCached;
};