#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "UIHelper.h"
#include "ScoreWidgetAnimator.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UScoreDisplayWidget::ScoreWidgetClass = nullptr;

UScoreDisplayWidget::UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 초기화
    PendingScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    bScoreTextActive = false;
    WidgetAnimator = nullptr;
}

UScoreDisplayWidget* UScoreDisplayWidget::CreateScoreWidget(UObject* WorldContextObject)
{
    // 기존 유효 인스턴스 확인
    if (IsInstanceValid())
    {
        return Instance;
    }
    
    // 리소스 확인
    APlayerController* Controller = GetValidPlayerController(WorldContextObject);
    if (!Controller) return nullptr;
    
    // 클래스 로드
    if (!LoadWidgetClassIfNeeded()) return nullptr;
    
    // 인스턴스 생성
    Instance = CreateWidget<UScoreDisplayWidget>(Controller, ScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10001);
        Instance->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    return Instance;
}

void UScoreDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 업데이트
    Instance = this;
    
    // 텍스트 블록 초기화
    InitializeTextBlocks();
    
    // 애니메이터 생성
    WidgetAnimator = NewObject<UScoreWidgetAnimator>(this, UScoreWidgetAnimator::StaticClass());
    WidgetAnimator->SetTextBlocks(ScoreTextBlock, ComboMultiplierTextBlock);
}

void UScoreDisplayWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

void UScoreDisplayWidget::InitializeTextBlocks()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패!"));
        return;
    }
    
    // 연한 노란색
    FLinearColor BrighterYellow = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    
    // 텍스트 블록 설정
    SetupTextBlock(ScoreTextBlock, BrighterYellow, 60, 750.0f, 100.0f);
    SetupTextBlock(ComboMultiplierTextBlock, BrighterYellow, 42, 800.0f, 200.0f);
    
    // 가로 직선 그림자 추가
    AddHorizontalShadow(ScoreTextBlock);
    AddHorizontalShadow(ComboMultiplierTextBlock);
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 텍스트 블록 및 그림자 설정 완료"));
}

void UScoreDisplayWidget::SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, float PosX, float PosY)
{
    UUIHelper::SetupTextBlockStyle(TextBlock, Color, FontSize, true);
    UUIHelper::SetScoreDisplayPosition(TextBlock, PosX, PosY, TextBlock == ScoreTextBlock ? 200.0f : 180.0f, TextBlock == ScoreTextBlock ? 80.0f : 70.0f, false);
}

void UScoreDisplayWidget::AddHorizontalShadow(UTextBlock* TextBlock)
{
    if (!TextBlock) return;
    
    // 텍스트 블록의 부모 패널 가져오기
    UPanelWidget* ParentPanel = TextBlock->GetParent();
    if (!ParentPanel) return;
    
    // 텍스트 블록 위치 및 크기 가져오기
    UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TextBlock->Slot);
    if (!TextSlot) return;
    
    FVector2D Position = TextSlot->GetPosition();
    FVector2D Size = TextSlot->GetSize();
    
    // 그림자 이미지 동적 생성
    UImage* ShadowImage = NewObject<UImage>(this, UImage::StaticClass(), FName(*FString::Printf(TEXT("%s_Shadow"), *TextBlock->GetName())));
    if (!ShadowImage) return;
    
    // 그림자 스타일 설정
    ShadowImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.3f)); // 반투명 검은색
    
    // 그림자 이미지를 컨테이너에 추가 (텍스트 블록 앞에 삽입하여 텍스트 뒤에 표시)
    ParentPanel->AddChild(ShadowImage);
    ParentPanel->RemoveChild(TextBlock);
    ParentPanel->AddChild(TextBlock); // 다시 추가하여 그림자 위에 텍스트 배치
    
    // 그림자 위치 설정 (텍스트 바로 아래 약간 더 넓게)
    UCanvasPanelSlot* ShadowSlot = Cast<UCanvasPanelSlot>(ShadowImage->Slot);
    if (ShadowSlot)
    {
        // 텍스트와 동일한 앵커 설정
        ShadowSlot->SetAnchors(TextSlot->GetAnchors());
        ShadowSlot->SetAlignment(TextSlot->GetAlignment());
        
        // 텍스트보다 약간 더 넓게, 그리고 아래에 배치
        float ShadowHeight = 4.0f; // 그림자 높이
        float WidthExtension = 10.0f; // 그림자 너비 확장
        
        ShadowSlot->SetPosition(FVector2D(Position.X - WidthExtension/2, Position.Y + Size.Y - ShadowHeight/2));
        ShadowSlot->SetSize(FVector2D(Size.X + WidthExtension, ShadowHeight));
    }
    
    // 그림자 슬롯을 먼저 배치하여 텍스트 뒤에 보이도록 함
    if (TextBlock->GetParent()->GetChildrenCount() > 0)
    {
        TextBlock->GetParent()->RemoveChild(ShadowImage);
        TextBlock->GetParent()->InsertChildAt(0, ShadowImage);
    }
}

bool UScoreDisplayWidget::IsInstanceValid()
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}

APlayerController* UScoreDisplayWidget::GetValidPlayerController(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: World가 유효하지 않음!"));
        return nullptr;
    }
    
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: PlayerController가 유효하지 않음!"));
        return nullptr;
    }
    
    return Controller;
}

bool UScoreDisplayWidget::LoadWidgetClassIfNeeded()
{
    if (ScoreWidgetClass)
    {
        return true;
    }
    
    static const TCHAR* BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C");
    ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, BlueprintPath);
    
    if (!ScoreWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("블루프린트를 찾을 수 없습니다: %s"), BlueprintPath);
        return false;
    }
    
    return true;
}

void UScoreDisplayWidget::DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier)
{
    UE_LOG(LogTemp, Warning, TEXT("DisplayScoreGain 호출됨: 점수=%d, 콤보=%d, 배율=%.1f"), 
           Score, ComboCount, ComboMultiplier);
    
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("텍스트 블록이 유효하지 않음!"));
        return;
    }
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World를 가져올 수 없음!"));
        return;
    }
    
    // 이전 애니메이션 취소
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
    }
    
    // 점수 텍스트 업데이트
    PendingScoreGain += Score;
    FString ScoreText = FString::Printf(TEXT("+%d"), PendingScoreGain);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    // 콤보 텍스트 업데이트
    CurrentComboMultiplier = ComboMultiplier;
    if (ComboCount >= 2)
    {
        FString ComboText = FString::Printf(TEXT("x%.1f"), CurrentComboMultiplier);
        ComboMultiplierTextBlock->SetText(FText::FromString(ComboText));
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 애니메이션 시작
    if (WidgetAnimator)
    {
        WidgetAnimator->StartFadeOutAnimation(this, 2.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("점수 표시 설정 완료: '%s'"), *ScoreText);
}