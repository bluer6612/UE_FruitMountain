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
    
    // 위젯 생성 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "UI|Score", meta = (WorldContext = "WorldContextObject"))
    static UScoreDisplayWidget* CreateScoreWidget(UObject* WorldContextObject);
    
    // 점수 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI|Score")
    void DisplayScoreGain(int32 Score, int32 ComboCount = 0, float ComboMultiplier = 1.0f);
    
protected:
    // UUserWidget 오버라이드
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // 텍스트 컴포넌트
    UPROPERTY()
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY()
    UTextBlock* ComboMultiplierTextBlock;
    
    // 부모 컴포넌트
    UPROPERTY()
    class UCanvasPanel* RootCanvas;
    
    // 애니메이션 타이머 핸들
    FTimerHandle ScoreAnimTimerHandle;
    
    // 점수 페이드 아웃 함수
    void FadeOutScoreText();
    
    // 새로 추가된 값을 기존 표시와 합치기 위한 변수
    int32 PendingScoreGain = 0;
    float CurrentComboMultiplier = 1.0f;
    bool bScoreTextActive = false;
};