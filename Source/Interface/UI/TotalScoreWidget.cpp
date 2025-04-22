#include "TotalScoreWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UIHelper.h"

// 정적 변수 초기화
UTotalScoreWidget* UTotalScoreWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UTotalScoreWidget::TotalScoreWidgetClass = nullptr;

// 색상 상수 정의
const FLinearColor UTotalScoreWidget::SCORE_BROWN_COLOR = FLinearColor(0.6f, 0.3f, 0.05f, 1.0f); // 갈색
const FLinearColor UTotalScoreWidget::SCORE_SHADOW_COLOR = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f); // 반투명 검은색

UTotalScoreWidget::UTotalScoreWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentTotalScore(0)
{
}

void UTotalScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 저장
    Instance = this;
    
    // 텍스트 스타일 설정
    SetupTotalScoreTextStyle();
    
    // 초기 총점 표시
    if (CurrentTotalScoreTextBlock)
    {
        CurrentTotalScoreTextBlock->SetText(FText::AsNumber(0));
    }
}

void UTotalScoreWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 인스턴스 정리
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

// 텍스트 스타일 설정 함수 구현
void UTotalScoreWidget::SetupTotalScoreTextStyle()
{
    if (!CurrentTotalScoreTextBlock)
    {
        return;
    }
    
    // UIHelper 함수를 사용하여 스타일 설정
    UUIHelper::SetupTextBlockStyle(
        CurrentTotalScoreTextBlock,
        SCORE_BROWN_COLOR,          // 갈색 텍스트
        32,                         // 폰트 크기
        true,                       // 그림자 사용
        SCORE_SHADOW_COLOR,         // 그림자 색상
        FVector2D(2.0f, 2.0f),      // 그림자 오프셋
        true,                       // 볼드체
        false,                      // 자동 줄바꿈 안함
        ESlateVisibility::Visible   // 기본 가시성
    );
}

UTotalScoreWidget* UTotalScoreWidget::CreateTotalScoreWidget(UObject* WorldContextObject)
{
    // UIHelper를 사용한 싱글톤 위젯 생성
    static const FString BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_TotalScore.BP_UI_Play_TotalScore_C");
    return UUIHelper::CreateSingletonWidget<UTotalScoreWidget>(Instance, TotalScoreWidgetClass, WorldContextObject, BlueprintPath, 10);
}

void UTotalScoreWidget::UpdateTotalScore(int32 NewScore)
{
    // 총점 업데이트
    CurrentTotalScore = NewScore;
    
    // UI 업데이트
    if (CurrentTotalScoreTextBlock)
    {
        // 점수 표시 형식 변경 (천 단위 구분 기호 추가)
        FNumberFormattingOptions NumberFormat;
        NumberFormat.UseGrouping = true;
        
        CurrentTotalScoreTextBlock->SetText(FText::AsNumber(CurrentTotalScore, &NumberFormat));
    }
}