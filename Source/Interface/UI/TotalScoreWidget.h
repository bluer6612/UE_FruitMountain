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
    
    // 위치 상수 - UI_Play_Score 위에 겹치도록 설정
    static const FVector2D TOTALSCORE_TEXT_POS;
    
    // 정적 인스턴스 - 싱글톤 패턴
    static UTotalScoreWidget* Instance;
    static TSubclassOf<UUserWidget> TotalScoreWidgetClass;
    
    // 위젯 생성 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static UTotalScoreWidget* CreateTotalScoreWidget(UObject* WorldContextObject);
    
    // 총점 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void UpdateTotalScore(int32 NewScore);
    
    // 점수 증가 애니메이션 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void AnimateScoreIncrease(int32 NewTotalScore);
    
    // 대기중인 점수 즉시 반영
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void ApplyPendingScore();
    
    // 정적 인스턴스 반환 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    static UTotalScoreWidget* GetInstance() { return Instance; }
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // 텍스트 블록 변수 추가 - 누락되었던 부분
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TotalScoreTextBlock;
    
    // 위젯 클래스 로드 헬퍼
    static bool LoadWidgetClassIfNeeded();
    
    // 유효한 플레이어 컨트롤러 획득
    static APlayerController* GetValidPlayerController(UObject* WorldContextObject);
    
    // 애니메이션 관련 변수들
    UPROPERTY()
    int32 CurrentTotalScore;
    
    UPROPERTY()
    int32 CurrentDisplayScore;
    
    UPROPERTY()
    int32 TargetScore;
    
    UPROPERTY()
    int32 StartScore;
    
    UPROPERTY()
    int32 PendingScore;
    
    UPROPERTY()
    int32 AnimSteps;
    
    UPROPERTY()
    int32 CurrentStep;
    
    UPROPERTY()
    bool bAnimating;
    
    // 애니메이션 타이머 핸들
    FTimerHandle ScoreAnimTimerHandle;
};