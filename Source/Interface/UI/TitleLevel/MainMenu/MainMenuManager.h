#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "TitleMenuManager.h"
#include "MainMenuManager.generated.h"

class UImage;
class UTitleLevelWidget;
class UMenuIndicatorAnimator;
class UStartMenuWidget;

UCLASS()
class UE_FRUITMOUNTAIN_API UMainMenuManager : public UTitleMenuManager
{
    GENERATED_BODY()
    
public:
    UMainMenuManager();

    // 초기화 메서드
    void Initialize(UImage* InSelectIndicator, UTitleLevelWidget* InOwner);
    
    // 키 입력 처리
    bool HandleKeyDown(const FKey& Key);
    
    // 메뉴 선택 관련 함수
    void MoveSelectionUp();
    void MoveSelectionDown();
    void SelectCurrentMenu();
    void UpdateMenuSelection();
    
    // 애니메이션 제어 함수 - 추가
    void StartIndicatorAnimation(bool bStart);
    
    // 소멸 시 정리
    virtual void BeginDestroy() override;
    
private:
    // 메뉴 동작 함수
    void OpenPlayLevel(); // 나중에 사용
    void OpenStartMenu(); // 새로 추가: 게임 모드 선택 메뉴 열기
    void OpenRankingMenu();
    void OpenOptionsMenu();
    void OpenCreditScreen();
    
    // 선택 효과 함수
    void PlaySelectionAnimation();

    // 메뉴 상태 변수
    int32 CurrentMenuIndex = 0;
    static constexpr int32 MenuItemCount = 4;
    
    // 참조 변수
    UPROPERTY()
    UImage* SelectIndicator = nullptr;
    
    UPROPERTY()
    UTitleLevelWidget* Owner = nullptr;
    
    // 애니메이션 관리자
    UPROPERTY()
    UMenuIndicatorAnimator* IndicatorAnimator = nullptr;
};