#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TotalScoreWidget.generated.h"

/**
 * 게임 전체 총점을 표시하는 위젯 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UTotalScoreWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UTotalScoreWidget(const FObjectInitializer& ObjectInitializer);
    
    // 총점 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void UpdateTotalScore(int32 NewScore);
    
    // 현재 총점 반환 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    int32 GetTotalScore() const { return CurrentTotalScore; }
    
    // 인스턴스 생성 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static UTotalScoreWidget* CreateTotalScoreWidget(UObject* WorldContextObject);
    
    // 인스턴스 접근자
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    static UTotalScoreWidget* GetInstance() { return Instance; }
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // 총점 텍스트 블록 - 블루프린트에서 바인딩
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentTotalScoreTextBlock;
    
    // 현재 총점
    UPROPERTY(BlueprintReadOnly, Category = "UI Score")
    int32 CurrentTotalScore;
    
    // 위젯 클래스 참조
    static TSubclassOf<UUserWidget> TotalScoreWidgetClass;
    
    // 싱글톤 인스턴스
    static UTotalScoreWidget* Instance;
    
    // 색상 상수 추가
    static const FLinearColor SCORE_BROWN_COLOR;
    static const FLinearColor SCORE_SHADOW_COLOR;
    
private:
    // 텍스트 스타일 설정 함수
    void SetupTotalScoreTextStyle();
};