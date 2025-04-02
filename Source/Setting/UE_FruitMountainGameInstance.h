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

    // UI 위젯 지속적 참조를 위한 변수
    UPROPERTY()
    UUserWidget* PersistentUIWidget;
    
    // UI 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowPersistentUI();

    UFUNCTION()
    void CheckPersistentUI();

    // 마지막으로 추가된 뷰포트 콘텐츠에 대한 참조
    TSharedPtr<SWidget> LastAddedViewportContent;
};