#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MenuIndicatorAnimator.generated.h"

class UImage;

/**
 * 메뉴 인디케이터 애니메이션을 담당하는 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UMenuIndicatorAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    UMenuIndicatorAnimator();
    
    // 초기화 함수
    void Initialize(UImage* InIndicator);
    
    // 애니메이션 시작/중지
    void EndAnimation();
    
    // 인디케이터 위치 변경
    void MoveToPosition(const FVector2D& NewPosition);
    
    // 객체 소멸 시 정리 작업
    virtual void BeginDestroy() override;
    
    UImage* GetIndicator() const { return Indicator; }

private:
    // 애니메이션 실행 함수
    void PlayAnimation();
    
    // 타이머 정리 함수
    void ClearAnimationTimers();
    
    // 애니메이션 상태 변수
    bool bIsAnimating = true;
    bool bIsAnimationRunning = false;
    
    // 애니메이션 타이머 관련 변수
    FTimerHandle AnimationTimerHandle;
    TArray<FTimerHandle> AnimationTimerHandles;
    float AnimationDuration = 2.75f;
    
    // 인디케이터 참조
    UPROPERTY()
    UImage* Indicator = nullptr;
};