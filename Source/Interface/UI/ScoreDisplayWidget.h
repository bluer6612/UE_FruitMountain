#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ScoreDisplayWidget.generated.h"

/**
 * 점수 및 콤보 배율을 화면에 표시하는 위젯
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UScoreDisplayWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // 정적 인스턴스 - 싱글톤 패턴
    static UScoreDisplayWidget* Instance;
    
    // 위젯 클래스 참조 (블루프린트 위젯 클래스)
    static TSubclassOf<UUserWidget> ScoreWidgetClass;
    
    // 위젯 생성 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static UScoreDisplayWidget* CreateScoreWidget(UObject* WorldContextObject);
    
    // 점수 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void DisplayScoreGain(int32 Score, int32 ComboCount = 0, float ComboMultiplier = 1.0f);
    
    // 테스트용 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static void ShowTestScore(UObject* WorldContextObject, int32 Score = 100);
    
protected:
    // UUserWidget 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // 블루프린트에서 노출된 변수들
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* ComboMultiplierTextBlock;
    
    // 선택적: 외부 블루프린트 접근을 위한 함수들
    UFUNCTION(BlueprintCallable, Category="UI Score")
    void SetScoreTextColor(FLinearColor Color) { if(ScoreTextBlock) ScoreTextBlock->SetColorAndOpacity(Color); }
    
    UFUNCTION(BlueprintCallable, Category="UI Score")
    void SetComboTextColor(FLinearColor Color) { if(ComboMultiplierTextBlock) ComboMultiplierTextBlock->SetColorAndOpacity(Color); }
    
    UFUNCTION(BlueprintCallable, Category="UI Score")
    void SetScoreTextSize(int32 Size) 
    { 
        if(ScoreTextBlock)
        {
            FSlateFontInfo FontInfo = ScoreTextBlock->GetFont();
            FontInfo.Size = Size;
            ScoreTextBlock->SetFont(FontInfo);
        }
    }
    
    // 애니메이션 타이머 핸들
    FTimerHandle ScoreAnimTimerHandle;
    
    // 점수 페이드 아웃 함수
    void FadeOutScoreText();
    
    // 새로 추가된 값을 기존 표시와 합치기 위한 변수
    int32 PendingScoreGain = 0;
    float CurrentComboMultiplier = 1.0f;
    bool bScoreTextActive = false;
};