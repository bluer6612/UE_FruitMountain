#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/UI/TitleLevel/Manager/TitleLevelWidget.h"
#include "StartMenuWidget.generated.h"


class UImage;
class UStartMenuManager;
class UMainMenuWidget;
class AFruitHUD;

UCLASS()
class UE_FRUITMOUNTAIN_API UStartMenuWidget : public UTitleLevelWidget
{
    GENERATED_BODY()

public:
    UStartMenuWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 초기화 함수
    void InitializeStartMenu();

    // 위젯 라이프사이클 함수
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 키 입력 처리 함수
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    
    // 게임 모드 관련 UI 요소
    UPROPERTY()
    class UImage* StartMenuImage;
    
    UPROPERTY()
    class UImage* SelectIndicator = nullptr;
    
    UPROPERTY()
    class UMenuIndicatorAnimator* IndicatorAnimator = nullptr;

    void BackToMainMenu();
    
protected:
    // 초기화
    void InitializeMenuManager();

private:
    // 페이드 효과 관련 변수 및 함수
    float FadeInDuration = 0.25f;
    bool bIsFading = false;
    
    // 메뉴 관리자
    UPROPERTY()
    class UStartMenuManager* MenuManager;
};