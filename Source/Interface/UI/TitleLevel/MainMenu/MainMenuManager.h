#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "MainMenuManager.generated.h"

class UImage;
class UMainMenuWidget;
class UMenuIndicatorAnimator;
class UStartMenuWidget;

UCLASS()
class UE_FRUITMOUNTAIN_API UMainMenuManager : public UObject
{
    GENERATED_BODY()
    
public:
    UMainMenuManager();

    // 초기화 메서드
    void Initialize(UMainMenuWidget* InOwner);
    
    // 키 입력 처리
    bool HandleKeyDown(const FKey& Key);
    
    // 메뉴 선택 관련 함수
    void MoveSelectionUp();
    void MoveSelectionDown();
    void SelectCurrentMenu();
    void UpdateMenuSelection();
    
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

    static constexpr int32 MenuItemCount = 4;
    
    UPROPERTY()
    class UMainMenuWidget* Owner = nullptr;
};