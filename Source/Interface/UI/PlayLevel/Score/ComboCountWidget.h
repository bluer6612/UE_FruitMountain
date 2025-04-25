#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ComboCountWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UComboCountWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UComboCountWidget(const FObjectInitializer& ObjectInitializer);
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void BeginDestroy() override;
    
    // 위젯 생성 함수
    static UComboCountWidget* CreateComboCountWidget(UObject* WorldContextObject);
    static UComboCountWidget* GetInstance() { return Instance; }
    
    // 인스턴스 유효성 검사
    static bool IsInstanceValid();

    static void ClearInstance();
    
    // 콤보 카운트 업데이트
    void UpdateComboCount(int32 NewComboCount);
    void ResetComboCount();
    
    // 위젯 가시성 조절
    void SetComboCountVisibility(bool bVisible);
    
private:
    // 싱글톤 인스턴스
    static UComboCountWidget* Instance;
    static TSubclassOf<UUserWidget> ComboCountWidgetClass;
    
    // UI 요소
    UPROPERTY()
    UImage* ComboImage;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ComboCountTextBlock;
    
    // 위치 및 색상 관련 상수
    static const FVector2D COMBO_IMAGE_POS;
    static const FVector2D COMBO_TEXT_POS;
    static const FLinearColor COMBO_TEXT_COLOR;
    
    // 상태 변수
    int32 CurrentComboCount;
    bool bAnimating;
    FTimerHandle ComboAnimTimerHandle;
    
    // 내부 함수
    void InitializeComboWidgets();
    static bool LoadWidgetClassIfNeeded();
    void PlayComboAnimation(int32 ComboCount);
    void ExecuteComboAnimation();
};