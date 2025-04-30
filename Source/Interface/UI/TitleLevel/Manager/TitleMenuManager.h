#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "TitleMenuManager.generated.h"

class UImage;
class UMenuIndicatorAnimator;

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleMenuManager : public UObject
{
    GENERATED_BODY()
public:
    // 인디케이터 애니메이션 및 위치 관리
    void InitializeIndicator(UImage* InSelectIndicator);
    void MoveIndicatorTo(const FVector2D& NewPosition);
    void StartIndicatorAnimation(bool bStart = true);

    // 방향키/선택 공통 처리
    bool HandleMenuKey(const FKey& Key, int32& InOutIndex, int32 ItemCount, TFunction<void()> OnSelect);

    // 페이드인 공통 처리
    void PlayFadeIn(UImage* TargetImage, UObject* WorldContext, float Duration = 0.25f);

protected:
    UPROPERTY()
    UMenuIndicatorAnimator* IndicatorAnimator = nullptr;
};