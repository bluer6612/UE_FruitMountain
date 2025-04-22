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
    
    // 애니메이션과 함께 점수 업데이트
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void AnimateScoreIncrease(int32 NewScore);
    
    // 대기중인 점수 즉시 반영
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void ApplyPendingScore();
    
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
    
    // 색상 상수
    static const FLinearColor SCORE_BROWN_COLOR;
    static const FLinearColor SCORE_SHADOW_COLOR;
    
private:
    // 위젯 클래스 로드 헬퍼
    static bool LoadWidgetClassIfNeeded();
    
    // 유효한 플레이어 컨트롤러 가져오기
    static APlayerController* GetValidPlayerController(UObject* WorldContextObject);
    
    // 텍스트 스타일 설정 함수
    void SetupTotalScoreTextStyle();
    
    // 위젯 위치 설정 함수
    void PositionWidgetAboveScoreDisplay();
    
    // 점수 증가 애니메이션 관련 멤버 변수
    UPROPERTY()
    FTimerHandle ScoreAnimTimerHandle;
    
    int32 TargetScore;
    int32 StartScore;
    int32 PendingScore;
    int32 AnimSteps;
    int32 CurrentStep;
    bool bAnimating;
    
    // 애니메이션 타이머 콜백
    void UpdateScoreAnimation();
};