#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleLevelWidget.generated.h"

class UImage;
class UTitleMenuManager;

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleLevelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UTitleLevelWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 초기화 함수
    void InitializeTitleWidget();

    // 위젯 라이프사이클 함수
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 헤더에 키 입력 처리 함수 추가
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    
    // 게임 UI 요소
    UPROPERTY()
    class UImage* LogoImage;
    
    UPROPERTY()
    class UImage* MenuImage;
    
    UPROPERTY()
    class UImage* SelectIndicator;
    
    // 게임 시작 함수 추가
    void StartGame();
    
protected:
    // 초기화
    void InitializeMenuManager();

private:
    // 페이드 효과 관련 변수 및 함수
    UPROPERTY(meta = (BindWidget))
    class UBorder* FadeBorder;
    
    float FadeOutDuration = 0.25f;
    float FadeTime = 0.0f;
    bool bIsFading = false;
    
    void PlayFadeOut();
    void PlayFadeIn(class UImage* TargetImage);
    void StartLogoAndMenuFadeIn();
    
    // 메뉴 관리자
    UPROPERTY()
    class UTitleMenuManager* MenuManager;
};