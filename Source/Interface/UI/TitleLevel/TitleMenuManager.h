#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "TitleMenuManager.generated.h"

class UImage;
class UTitleLevelWidget;

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
    
    // 메뉴 선택 위치 업데이트
    void UpdateMenuSelection();
    
private:
    // 메뉴 선택 관련 함수
    void MoveSelectionUp();
    void MoveSelectionDown();
    void SelectCurrentMenu();
    void PlaySelectionAnimation();
    
    // 메뉴 아이템 처리 함수
    void OpenPlayLevel();
    void OpenRankingMenu();
    void OpenOptionsMenu();
    void OpenCreditScreen();
    
    // 메뉴 상태 변수
    int32 CurrentMenuIndex = 0;
    static constexpr int32 MenuItemCount = 4;
    float MenuPositions[MenuItemCount] = { 50.0f, 100.0f, 150.0f, 200.0f };
    
    // 참조 변수
    UPROPERTY()
    UImage* SelectIndicator = nullptr;
    
    UPROPERTY()
    UTitleLevelWidget* Owner = nullptr;
    
    // 선택 표시기 애니메이션 관련 변수
    FTimerHandle IndicatorAnimationTimerHandle;
    float IndicatorAnimationDuration = 0.8f;  // 애니메이션 지속 시간
    float IndicatorAnimationInterval = 1.5f;  // 애니메이션 간격
    
    // 선택 표시기 애니메이션 함수
    void StartIndicatorAnimation(float );
    void PlayIndicatorAnimation();
};