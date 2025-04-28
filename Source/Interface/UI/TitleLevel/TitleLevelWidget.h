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

    // 메뉴 인덱스(0: 시작, 1: 종료 등)
    int32 CurrentMenuIndex = 0;

    void InitializeTitleWidget();

    // 위젯 라이프사이클 함수
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    UPROPERTY()
    class UBorder* FadeBorder = nullptr;

    void PlayFadeOut(class UBorder* TargetBorder, float Duration);
    void PlayFadeIn(class UImage* TargetImage);

    // Logo/Menu 이미지 포인터
    UPROPERTY()
    class UImage* LogoImage = nullptr;
    UPROPERTY()
    class UImage* MenuImage = nullptr;

    void UpdateMenuSelection();
    void OnMenuSelect();

private:
    bool bIsFading = false;
    float FadeTime = 0.0f;
    float FadeOutDuration = 0.0f; // 페이드 아웃 지속 시간

    void StartLogoAndMenuFadeIn();
};