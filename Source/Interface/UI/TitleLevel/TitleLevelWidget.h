#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleLevelWidget.generated.h"

UCLASS()
class INTERFACE_API UTitleLevelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UTitleLevelWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 초기화 함수
    void InitializeTitleWidget();
    
protected:
    // 키 입력은 여전히 이 클래스에서 받아야 함 (가상 함수)
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    
private:
    // 페이드 효과 관련 변수 및 함수
    UPROPERTY(meta = (BindWidget))
    class UBorder* FadeBorder;
    
    float FadeOutDuration = 2.5f;
    float FadeTime = 0.0f;
    bool bIsFading = false;
    
    void PlayFadeOut();
    void PlayFadeIn(class UImage* TargetImage);
    void StartLogoAndMenuFadeIn();
    
    // 게임 UI 요소
    UPROPERTY()
    class UImage* LogoImage;
    
    UPROPERTY()
    class UImage* MenuImage;
    
    UPROPERTY()
    class UImage* SelectIndicator;
    
    // 메뉴 관리자
    UPROPERTY()
    class UTitleMenuManager* MenuManager;
    
    // 메뉴 위치 배열 (UI_Title_Menu 상단에서 50f씩 증가)
    float MenuPositions[4] = { 50.0f, 100.0f, 150.0f, 200.0f };
};