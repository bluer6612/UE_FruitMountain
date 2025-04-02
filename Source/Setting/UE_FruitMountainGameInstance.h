#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UE_FruitMountainGameInstance.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UUE_FruitMountainGameInstance : public UGameInstance
{
    GENERATED_BODY()
    
public:
    virtual void Init() override;
    
    // UI 테스트 함수 추가
    UFUNCTION(BlueprintCallable, Category = "UI")
    void TestCreateSimpleUI();
};