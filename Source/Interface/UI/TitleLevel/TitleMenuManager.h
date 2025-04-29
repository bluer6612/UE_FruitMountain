#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "TitleMenuManager.generated.h"

class UImage;
class UTitleLevelWidget;
class UMenuIndicatorAnimator;

/**
 * 타이틀 메뉴 관리자
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UTitleMenuManager : public UObject
{
    GENERATED_BODY()
    
public:
    UTitleMenuManager();

    // 초기화 메서드
    void Initialize(UImage* InSelectIndicator, UTitleLevelWidget* InOwner);
    
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
    void OpenPlayLevel();
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