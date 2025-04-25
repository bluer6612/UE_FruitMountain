#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ComboCountWidget.generated.h"

class UComboCountWidgetAnimator;

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
    static void ClearInstance();
    static bool IsInstanceValid();
    
    // 애니메이터 접근 함수 추가
    UComboCountWidgetAnimator* GetAnimator() const { return WidgetAnimator; }
    
    // 상태 접근 함수 추가
    void SetCurrentComboCount(int32 Value) { CurrentComboCount = Value; }
    int32 GetCurrentComboCount() const { return CurrentComboCount; }
    
private:
    // 싱글톤 인스턴스
    static UComboCountWidget* Instance;
    static TSubclassOf<UUserWidget> ComboCountWidgetClass;
    
    // UI 요소
    UPROPERTY()
    UImage* ComboCountImage;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ComboCountTextBlock;
    
    // 애니메이션 컨트롤러
    UPROPERTY()
    UComboCountWidgetAnimator* WidgetAnimator;
    
    // 위치 및 색상 관련 상수
    static const FVector2D COMBOCOUNT_IMAGE_POS;
    static const FVector2D COMBOCOUNT_TEXT_POS;
    static const FLinearColor COMBOCOUNT_TEXT_COLOR;
    
    // 상태 변수
    int32 CurrentComboCount;
    
    // 내부 함수
    void InitializeComboWidgets();
    static bool LoadWidgetClassIfNeeded();
};