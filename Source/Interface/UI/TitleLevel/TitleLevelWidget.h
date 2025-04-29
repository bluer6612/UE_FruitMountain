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

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    void InitializeTitleWidget();

    float FadeOutDuration = 0.0f;

protected:
    void PlayFadeOut();
    void PlayFadeIn(class UImage* TargetImage);

    // Logo/Menu 이미지 포인터
    UPROPERTY()
    class UImage* LogoImage = nullptr;
    UPROPERTY()
    class UImage* MenuImage = nullptr;
    
    UPROPERTY(meta = (BindWidget))
    class UBorder* FadeBorder;

    void UpdateMenuSelection();
    void OnMenuSelect();

    void StartLogoAndMenuFadeIn();

private:
    // 메뉴 관련 변수
    int32 CurrentMenuIndex = 0;
    int32 MenuItemCount = 4;       // 4개 메뉴 슬롯
    
// 선택 표시기
    UPROPERTY()
    class UImage* SelectIndicator;
    
    // 메뉴 항목 Y 위치 배열
    float MenuPositions[4] = { 50.0f, 100.0f, 150.0f, 200.0f };
    
    bool bIsFading = false;
    float FadeTime = 0.0f;
};