#include "TotalScoreWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "UIHelper.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TextureDisplayWidget.h"

// 정적 멤버 초기화
UTotalScoreWidget* UTotalScoreWidget::Instance = nullptr;

// UI_Play_Score 위젯 위에 겹치게 위치 설정
const FVector2D UTotalScoreWidget::TOTALSCORE_TEXT_POS = FVector2D(292.0f, 120.0f);

UTotalScoreWidget::UTotalScoreWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentTotalScore(0)
    , TargetScore(0)
    , StartScore(0)
    , PendingScore(0)
    , AnimSteps(25)  // 애니메이션 단계 수
    , CurrentStep(0)
    , bAnimating(false)
{
}

void UTotalScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 설정
    Instance = this;
    
    // 총점 텍스트 블록이 있는지 확인
    if (TotalScoreTextBlock)
    {
        // 위치 설정
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(TotalScoreTextBlock->Slot))
        {
            // 고정된 위치 사용 (UI_Play_Score 위에 겹치게)
            ScoreSlot->SetPosition(TOTALSCORE_TEXT_POS);
            
            // 크기와 정렬 설정
            ScoreSlot->SetSize(FVector2D(300.0f, 60.0f));
            ScoreSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬
        }
        
        // 초기 텍스트 설정
        TotalScoreTextBlock->SetText(FText::FromString(TEXT("0")));
        
        // 폰트 크기와 스타일 설정
        TotalScoreTextBlock->SetColorAndOpacity(FLinearColor(0.35f, 0.2f, 0.05f, 1.0f)); // 어두운 갈색
        TotalScoreTextBlock->SetFont(FSlateFontInfo(TotalScoreTextBlock->GetFont().FontObject, 36.0f));
    }
}

void UTotalScoreWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
    }
    
    // 인스턴스 정리
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

UTotalScoreWidget* UTotalScoreWidget::CreateTotalScoreWidget(UObject* WorldContextObject)
{
    // 기존 유효 인스턴스 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 확인
    APlayerController* PC = GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }
    
    // 위젯 클래스 로드
    if (!LoadWidgetClassIfNeeded())
    {
        return nullptr;
    }
    
    // 위젯 생성
    Instance = CreateWidget<UTotalScoreWidget>(PC, TotalScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10);
    }
    
    return Instance;
}

bool UTotalScoreWidget::LoadWidgetClassIfNeeded()
{
    if (!TotalScoreWidgetClass)
    {
        TotalScoreWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_TotalScore.BP_UI_Play_TotalScore_C"));
    }
    return TotalScoreWidgetClass != nullptr;
}

APlayerController* UTotalScoreWidget::GetValidPlayerController(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    return World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
}

void UTotalScoreWidget::UpdateTotalScore(int32 NewScore)
{
    // 점수 업데이트
    CurrentTotalScore = NewScore;
    
    // UI 업데이트
    if (TotalScoreTextBlock)
    {
        // 점수 표시 형식 변경 (천 단위 구분 기호 추가)
        FNumberFormattingOptions NumberFormat;
        NumberFormat.UseGrouping = true;
        
        TotalScoreTextBlock->SetText(FText::AsNumber(CurrentTotalScore, &NumberFormat));
    }
}

// 애니메이션과 함께 점수 증가
void UTotalScoreWidget::AnimateScoreIncrease(int32 NewTotalScore)
{
    if (!TotalScoreTextBlock || !IsValid(TotalScoreTextBlock))
    {
        UE_LOG(LogTemp, Error, TEXT("TotalScoreTextBlock이 유효하지 않습니다"));
        return;
    }
    
    // 기존 점수와 목표 점수 설정
    CurrentDisplayScore = FCString::Atoi(*TotalScoreTextBlock->GetText().ToString());
    TargetScore = NewTotalScore;
    
    // 애니메이션 적용: 점수가 증가하는 효과
    if (TargetScore > CurrentDisplayScore)
    {
        // 타이머 설정 (매 프레임마다 호출)
        UWorld* World = GetWorld();
        if (World)
        {
            FTimerDelegate ScoreTickDelegate;
            ScoreTickDelegate.BindLambda([this]()
            {
                // 점수를 매 프레임마다 조금씩 증가
                int32 Increment = FMath::Max(1, (TargetScore - CurrentDisplayScore) / 10);
                CurrentDisplayScore = FMath::Min(TargetScore, CurrentDisplayScore + Increment);
                
                // 텍스트 업데이트
                TotalScoreTextBlock->SetText(FText::FromString(FString::FromInt(CurrentDisplayScore)));
                
                // 목표 점수에 도달하면 타이머 종료
                if (CurrentDisplayScore >= TargetScore)
                {
                    GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
                }
                
                // 숫자가 변할 때 약간의 크기 애니메이션도 적용
                float Scale = 1.0f + (0.2f * (float)(CurrentDisplayScore - TargetScore) / (float)TargetScore);
                TotalScoreTextBlock->SetRenderScale(FVector2D(Scale, Scale));
            });
            
            // 0.02초마다 업데이트
            World->GetTimerManager().SetTimer(ScoreAnimTimerHandle, ScoreTickDelegate, 0.02f, true);
        }
    }
    else
    {
        // 즉시 업데이트
        TotalScoreTextBlock->SetText(FText::FromString(FString::FromInt(NewTotalScore)));
    }
}

// 대기중인 점수 즉시 반영
void UTotalScoreWidget::ApplyPendingScore()
{
    // 애니메이션 종료 및 점수 바로 반영
    if (bAnimating)
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
        bAnimating = false;
    }
    
    UpdateTotalScore(PendingScore);
}