#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "TitleMenuManager.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleMenuManager : public UObject
{
    GENERATED_BODY()
    
public:
    UTitleMenuManager();
    
    // 메뉴 초기화
    void Initialize(class UImage* InSelectIndicator, class UTitleLevelWidget* InOwner);
    
    // 키 입력 처리 - NativeOnKeyDown에서 호출될 함수
    bool HandleKeyDown(const FKey& Key);
    
    // 메뉴 인덱스 관리
    void MoveSelectionUp();
    void MoveSelectionDown();
    void SelectCurrentMenu();
    
    // 메뉴 위치 업데이트
    void UpdateMenuSelection();

private:
    // 메뉴 관련 변수
    int32 CurrentMenuIndex = 0;
    int32 MenuItemCount = 4;
    
    // 선택 표시기 참조
    UPROPERTY()
    class UImage* SelectIndicator = nullptr;
    
    // 소유자 참조 (TitleLevelWidget)
    UPROPERTY()
    class UTitleLevelWidget* Owner = nullptr;
    
    // 선택 효과를 위한 애니메이션
    void PlaySelectionAnimation();
    
    // 메뉴 항목별 처리 함수
    void OpenPlayLevel();
    void OpenRankingMenu();
    void OpenOptionsMenu();
    void OpenCreditScreen();
};