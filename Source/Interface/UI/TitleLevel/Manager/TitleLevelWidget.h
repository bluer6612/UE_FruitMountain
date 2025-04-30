#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "TitleLevelWidget.generated.h"

class UImage;
class UMainMenuManager;

UCLASS()
class UE_FRUITMOUNTAIN_API UTitleLevelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UBorder* FadeBorder;

    // 페이드 효과 관련 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fade")
    float FadeOutDuration = 0.25f;

    UFUNCTION(BlueprintCallable)
    void PlayFadeIn(UImage* TargetImage, float Duration = 0.25f);

    UFUNCTION(BlueprintCallable)
    void PlayFadeOut();

protected:
    virtual void NativeConstruct() override;
};