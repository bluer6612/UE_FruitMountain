#include "TotalScoreWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "UIHelper.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TextureDisplayWidget.h"

// 정적 멤버 초기화
UTotalScoreWidget* UTotalScoreWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UTotalScoreWidget::TotalScoreWidgetClass = nullptr;

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
    
    // 기존 인스턴스가 있으면 정리
    if (Instance && Instance != this)
    {
        Instance->RemoveFromParent();
    }
    
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
            ScoreSlot->SetAlignment(FVector2D(0.0f, 0.5f)); // 왼쪽 정렬 (수직으로는 중앙)
        }
        
        // 초기 텍스트 설정
        TotalScoreTextBlock->SetText(FText::FromString(TEXT("0")));
        
        // 폰트 크기와 스타일 설정
        TotalScoreTextBlock->SetColorAndOpacity(FLinearColor(0.15f, 0.075f, 0.05f, 1.0f)); // 어두운 갈색

        // FSlateFontInfo 생성자 문법 수정
        FSlateFontInfo FontInfo = TotalScoreTextBlock->GetFont();
        FontInfo.Size = 45.0f;
        TotalScoreTextBlock->SetFont(FontInfo);
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
    
    // 인스턴스 정리 - 명시적으로 참조 제거
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
        // 먼저 뷰포트에 추가
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
    
    // 기존 애니메이션 취소
    if (bAnimating && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
        bAnimating = false;
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
            // 약한 참조로 람다 함수 캡처 - GC 안전성 확보
            TWeakObjectPtr<UTotalScoreWidget> WeakThis(this);
            
            FTimerDelegate ScoreTickDelegate;
            ScoreTickDelegate.BindLambda([WeakThis]()
            {
                // 약한 참조를 통해 유효성 검사
                if (!WeakThis.IsValid())
                {
                    return;
                }
                
                UTotalScoreWidget* Self = WeakThis.Get();
                
                // 점수를 매 프레임마다 조금씩 증가
                int32 Increment = FMath::Max(1, (Self->TargetScore - Self->CurrentDisplayScore) / 10);
                Self->CurrentDisplayScore = FMath::Min(Self->TargetScore, Self->CurrentDisplayScore + Increment);
                
                // 텍스트 업데이트 - 천 단위 구분자 추가
                FNumberFormattingOptions NumberFormat;
                NumberFormat.UseGrouping = true;
                Self->TotalScoreTextBlock->SetText(FText::AsNumber(Self->CurrentDisplayScore, &NumberFormat));
                
                // 목표 점수에 도달하면 타이머 종료
                if (Self->CurrentDisplayScore >= Self->TargetScore)
                {
                    if (Self->GetWorld())
                    {
                        Self->GetWorld()->GetTimerManager().ClearTimer(Self->ScoreAnimTimerHandle);
                        Self->bAnimating = false;
                    }
                }
                
                // 숫자가 변할 때 약간의 크기 애니메이션도 적용
                float Scale = 1.0f + (0.1f * (float)(Self->TargetScore - Self->CurrentDisplayScore) / (float)Self->TargetScore);
                Self->TotalScoreTextBlock->SetRenderScale(FVector2D(Scale, Scale));
            });
            
            // 0.02초마다 업데이트
            bAnimating = true;
            World->GetTimerManager().SetTimer(ScoreAnimTimerHandle, ScoreTickDelegate, 0.02f, true);
        }
    }
    else
    {
        // 즉시 업데이트
        FNumberFormattingOptions NumberFormat;
        NumberFormat.UseGrouping = true;
        TotalScoreTextBlock->SetText(FText::AsNumber(NewTotalScore, &NumberFormat));
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