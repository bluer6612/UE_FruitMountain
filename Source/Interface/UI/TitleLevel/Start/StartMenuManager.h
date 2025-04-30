#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "StartMenuManager.generated.h"

class UImage;
class UStartMenuWidget;
class UMenuIndicatorAnimator;

/**
 * 게임 모드 선택 메뉴 관리자
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UStartMenuManager : public UObject
{
    GENERATED_BODY()
    
public:
    UStartMenuManager();

    // 초기화 메서드
    void Initialize(UImage* InSelectIndicator, UStartMenuWidget* InOwner);
    
    // 키 입력 처리
    bool HandleKeyDown(const FKey& Key);
    
    // 메뉴 선택 관련 함수
    void MoveSelectionUp();
    void MoveSelectionDown();
    void SelectCurrentMenu();
    void UpdateMenuSelection();
    
    // 애니메이션 제어 함수
    void StartIndicatorAnimation(bool bStart = true);
    
    // 소멸 시 정리
    virtual void BeginDestroy() override;
    
private:
    // 메뉴 동작 함수
    void SelectClassicMode();
    void SelectTimeLimitMode();
    void BackToMainMenu();
    
    // 선택 효과 함수
    void PlaySelectionAnimation();
    
    // 게임 모드 설명 이미지 업데이트
    void UpdateGameModeDescription();

    // 메뉴 상태 변수
    int32 CurrentMenuIndex = 0;
    static constexpr int32 MenuItemCount = 3; // 기본 모드, 시간 제한 모드, 뒤로가기
    
    // 참조 변수
    UPROPERTY()
    UImage* SelectIndicator = nullptr;
    
    UPROPERTY()
    UStartMenuWidget* Owner = nullptr;
    
    // 애니메이션 관리자
    UPROPERTY()
    UMenuIndicatorAnimator* IndicatorAnimator = nullptr;
};