#include "ComboCountWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "TimerManager.h"

// 정적 멤버 초기화
UComboCountWidget* UComboCountWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UComboCountWidget::ComboCountWidgetClass = nullptr;

// 위치 및 색상 상수 초기화 - 화면 중앙과 상단 사이에 위치
const FVector2D UComboCountWidget::COMBO_IMAGE_POS = FVector2D(512.0f, 240.0f);
const FVector2D UComboCountWidget::COMBO_TEXT_POS = FVector2D(540.0f, 265.0f);
const FLinearColor UComboCountWidget::COMBO_TEXT_COLOR = FLinearColor(1.0f, 1.0f, 0.4f, 1.0f);

UComboCountWidget::UComboCountWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentComboCount(0)
    , bAnimating(false)
{
}

void UComboCountWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 기존 인스턴스가 있으면 정리
    if (Instance && Instance != this)
    {
        Instance->RemoveFromParent();
    }
    
    // 인스턴스 설정
    Instance = this;
    
    // 콤보 위젯 초기화
    InitializeComboWidgets();
    
    // 초기에는 숨김 상태로 시작
    SetComboCountVisibility(false);
}

void UComboCountWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 애니메이션 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
    }
    
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

void UComboCountWidget::BeginDestroy()
{
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    Super::BeginDestroy();
}

UComboCountWidget* UComboCountWidget::CreateComboCountWidget(UObject* WorldContextObject)
{
    // 이미 유효한 인스턴스가 있는지 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 기존 인스턴스 정리
    if (Instance)
    {
        Instance = nullptr;
    }
    
    // 위젯 클래스 로드
    if (!LoadWidgetClassIfNeeded())
    {
        return nullptr;
    }
    
    APlayerController* Controller = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 유효한 플레이어 컨트롤러가 없습니다"));
        return nullptr;
    }
    
    // 위젯 생성 및 뷰포트에 추가
    Instance = CreateWidget<UComboCountWidget>(Controller, ComboCountWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10002); // 항상 최상위에 표시 (ScoreDisplayWidget보다 높은 Z-Order)
        Instance->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    return Instance;
}

bool UComboCountWidget::LoadWidgetClassIfNeeded()
{
    if (!ComboCountWidgetClass)
    {
        ComboCountWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_ComboCount.BP_UI_Play_ComboCount_C"));
    }
    return ComboCountWidgetClass != nullptr;
}

void UComboCountWidget::InitializeComboWidgets()
{
    // 이미지와 텍스트 블록이 이미 블루프린트에 설정되어 있는지 확인
    if (!ComboImage)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComboImage가 설정되지 않았습니다. 수동으로 생성합니다."));
        
        // 루트 패널 찾기
        UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
        if (!RootPanel)
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 루트 패널을 찾을 수 없습니다."));
            return;
        }
        
        // 이미지 로드
        UTexture2D* ComboTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/PlayLevel/UI_Play_ComboCount"));
        if (!ComboTexture)
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 콤보 텍스처 로드 실패"));
            return;
        }
        
        // 이미지 위젯 생성
        ComboImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (ComboImage)
        {
            ComboImage->SetBrushFromTexture(ComboTexture);
            
            // 패널에 추가
            UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(RootPanel->AddChild(ComboImage));
            if (ImageSlot)
            {
                ImageSlot->SetPosition(COMBO_IMAGE_POS);
                ImageSlot->SetSize(FVector2D(300.0f, 100.0f)); // 적절한 크기로 조정
                ImageSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬
            }
        }
    }
    
    if (!ComboCountTextBlock)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComboCountTextBlock이 설정되지 않았습니다. 수동으로 생성합니다."));
        
        // 루트 패널 찾기
        UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
        if (!RootPanel)
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 루트 패널을 찾을 수 없습니다."));
            return;
        }
        
        // 텍스트 블록 생성
        ComboCountTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (ComboCountTextBlock)
        {
            // 스타일 설정
            UUIWidgetUtility::SetupTextBlockStyle(
                ComboCountTextBlock,
                COMBO_TEXT_COLOR,
                48.0f, // 큰 폰트 크기
                UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH,
                true,  // 볼드체
                false, // 자동 줄바꿈 안 함
                ESlateVisibility::HitTestInvisible
            );
            
            // 텍스트 초기화
            ComboCountTextBlock->SetText(FText::FromString(TEXT("0")));
            
            // 패널에 추가
            UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(RootPanel->AddChild(ComboCountTextBlock));
            if (TextSlot)
            {
                TextSlot->SetPosition(COMBO_TEXT_POS);
                TextSlot->SetSize(FVector2D(100.0f, 50.0f));
                TextSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬
            }
        }
    }
}

void UComboCountWidget::UpdateComboCount(int32 NewComboCount)
{
    // 이전 콤보 카운트와 다르면 애니메이션 재생
    if (CurrentComboCount != NewComboCount)
    {
        CurrentComboCount = NewComboCount;
        PlayComboAnimation(CurrentComboCount);
    }
    
    // 콤보가 2 이상일 때만 보이게
    if (CurrentComboCount >= 2)
    {
        SetComboCountVisibility(true);
        
        // 텍스트 업데이트
        if (ComboCountTextBlock)
        {
            ComboCountTextBlock->SetText(FText::AsNumber(CurrentComboCount));
        }
    }
    else
    {
        SetComboCountVisibility(false);
    }
}

void UComboCountWidget::SetComboCountVisibility(bool bVisible)
{
    ESlateVisibility InVisibility = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
    
    if (ComboImage)
    {
        ComboImage->SetVisibility(InVisibility);
    }
    
    if (ComboCountTextBlock)
    {
        ComboCountTextBlock->SetVisibility(InVisibility);
    }
}

void UComboCountWidget::PlayComboAnimation(int32 ComboCount)
{
    // 이미 애니메이션 중이면 취소
    if (bAnimating && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        bAnimating = false;
    }
    
    // 콤보가 없으면 애니메이션 필요 없음
    if (ComboCount < 2)
    {
        return;
    }
    
    // 애니메이션 시작
    bAnimating = true;
    
    // 초기 크기 설정 (약간 크게 시작)
    if (ComboCountTextBlock)
    {
        ComboCountTextBlock->SetRenderScale(FVector2D(1.5f, 1.5f));
    }
    
    if (ComboImage)
    {
        ComboImage->SetRenderScale(FVector2D(1.3f, 1.3f));
    }
    
    // 타이머로 애니메이션 실행
    FTimerDelegate AnimDelegate;
    AnimDelegate.BindUObject(this, &UComboCountWidget::ExecuteComboAnimation);
    
    GetWorld()->GetTimerManager().SetTimer(ComboAnimTimerHandle, AnimDelegate, 0.016f, true); // ~60fps
}

void UComboCountWidget::ExecuteComboAnimation()
{
    static int32 AnimSteps = 0;
    static const int32 MaxSteps = 15; // 애니메이션 총 프레임 수
    
    AnimSteps++;
    
    // 애니메이션 진행률 (0.0 ~ 1.0)
    float Progress = FMath::Clamp((float)AnimSteps / (float)MaxSteps, 0.0f, 1.0f);
    
    // 이징 함수 - 시작은 빠르게, 끝은 부드럽게
    float EasedProgress = FMath::CubicInterp(0.0f, 0.0f, 1.0f, 0.0f, Progress);
    
    // 크기 애니메이션 (1.5/1.3배 -> 1.0배)
    float TextScale = 1.0f + 0.5f * (1.0f - Progress);
    float ImageScale = 1.0f + 0.3f * (1.0f - Progress);
    
    if (ComboCountTextBlock)
    {
        ComboCountTextBlock->SetRenderScale(FVector2D(TextScale, TextScale));
    }
    
    if (ComboImage)
    {
        ComboImage->SetRenderScale(FVector2D(ImageScale, ImageScale));
    }
    
    // 애니메이션 종료 체크
    if (AnimSteps >= MaxSteps)
    {
        // 타이머 중지
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        AnimSteps = 0;
        bAnimating = false;
        
        // 최종 크기 리셋
        if (ComboCountTextBlock)
        {
            ComboCountTextBlock->SetRenderScale(FVector2D(1.0f, 1.0f));
        }
        
        if (ComboImage)
        {
            ComboImage->SetRenderScale(FVector2D(1.0f, 1.0f));
        }
    }
}

void UComboCountWidget::ResetComboCount()
{
    CurrentComboCount = 0;
    SetComboCountVisibility(false);
    
    // 애니메이션 취소
    if (bAnimating && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboAnimTimerHandle);
        bAnimating = false;
    }
}

bool UComboCountWidget::IsInstanceValid()
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}

void UComboCountWidget::ClearInstance()
{
    if (Instance)
    {
        if (Instance->IsInViewport())
        {
            Instance->RemoveFromParent();
        }
        Instance = nullptr;
    }
}