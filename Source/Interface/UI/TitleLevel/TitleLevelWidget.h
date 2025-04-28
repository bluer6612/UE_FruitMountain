#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleLevelWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleLevelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UTitleLevelWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // 메뉴 인덱스(0: 시작, 1: 종료 등)
    int32 CurrentMenuIndex = 0;

protected:
    UPROPERTY()
    class UImage* LogoImage;

    UPROPERTY()
    class UImage* MenuImage;

    void UpdateMenuSelection();
    void OnMenuSelect();
};