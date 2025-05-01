#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Templates/Function.h"
#include "TitleLevelWidget.generated.h"

class UImage;
class UMainMenuManager;

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleLevelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 페이드 효과 관련 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fade")
    float FadeOutDuration = 0.25f;

    UFUNCTION(BlueprintCallable)
    void PlayFadeIn(UImage* TargetImage, float Duration = 0.25f);

    UFUNCTION(BlueprintCallable)
    void PlayFadeOut();

    bool HandleMenuKey(const FKey& Key, int32& InOutIndex, int32 ItemCount, TFunction<void()> OnSelect);
    
    UFUNCTION(BlueprintCallable)
    void StartGame();
    
    int CurrentMenuIndex = 0;

    // 게임 UI 요소
    UPROPERTY(meta = (BindWidget))
    UBorder* TitleFadeBorder;

protected:
    /// 페이드 인 최초 호출 여부
    bool bLogoFadeInCalled = false;
};