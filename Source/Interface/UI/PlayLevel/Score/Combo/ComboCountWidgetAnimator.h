#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ComboCountWidgetAnimator.generated.h"

// 애니메이션 완료 델리게이트
DECLARE_DELEGATE(FOnAnimationComplete);

UCLASS()
class UE_FRUITMOUNTAIN_API UComboCountWidgetAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    UComboCountWidgetAnimator();
    virtual void BeginDestroy() override;
    
    // 애니메이터 초기화 함수
    void Initialize(UImage* InComboCountImage, UTextBlock* InComboTextBlock);
    
    // 위젯 컨트롤 인터페이스 - ComboCountWidget에서 이동
    void UpdateComboCount(int32 NewComboCount);
    void ResetComboCount();
    void SetComboCountVisibility(bool bVisible);
    
    // 애니메이션 인터페이스
    void PlayFadeOutAnimation();
    void CancelAnimation();
    
    // 이벤트 콜백 속성
    FOnAnimationComplete OnAnimationComplete;
    
private:
    // 참조 위젯
    UPROPERTY()
    UImage* ComboCountImage;
    
    UPROPERTY()
    UTextBlock* ComboTextBlock;
    
    // 상태 변수
    int32 CurrentComboCount;
    bool bFadingOut;
    float FadeOutDuration;
    FTimerHandle FadeOutTimerHandle;
    
    // 애니메이션 실행 함수
    void ExecuteFadeOutStep();
    void OnAnimationCompleted();
};