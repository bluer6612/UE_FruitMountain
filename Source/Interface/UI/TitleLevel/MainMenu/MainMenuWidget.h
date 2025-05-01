#pragma once

#include "CoreMinimal.h"
#include "Interface/UI/TitleLevel/Manager/TitleLevelWidget.h"
#include "MainMenuWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UMainMenuWidget : public UTitleLevelWidget
{
    GENERATED_BODY()

public:
    UMainMenuWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 초기화 함수
    void InitializeMainMenuWidget();
    
    void StartLogoAndMenuFadeIn();

    // 위젯 라이프사이클 함수
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 헤더에 키 입력 처리 함수 추가
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    
    // 게임 UI 요소
    UPROPERTY(meta = (BindWidget))
    UBorder* FadeBorder;

    UPROPERTY()
    class UImage* LogoImage;
    
    UPROPERTY()
    class UImage* MenuImage;
    
    UPROPERTY()
    class UImage* SelectIndicator = nullptr;

    UPROPERTY()
    class UMenuIndicatorAnimator* IndicatorAnimator = nullptr;
protected:
    // 초기화
    void InitializeMenuManager();

private:
    // 메뉴 관리자
    UPROPERTY()
    class UMainMenuManager* MenuManager;
};