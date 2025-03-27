#pragma once

#include "CoreMinimal.h"
#include "GameFramework/InputSettings.h"
#include "FruitInputMappingManager.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitInputMappingManager : public UObject
{
    GENERATED_BODY()

public:
    // 입력 매핑 설정 실행 함수
    UFUNCTION(BlueprintCallable, Category = "Input")
    static void ConfigureKeyMappings();
};