#include "ComboCountWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "TimerManager.h"
#include "ComboCountWidgetAnimator.h"

// 정적 멤버 초기화
UComboCountWidget* UComboCountWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UComboCountWidget::ComboCountWidgetClass = nullptr;

// 위치 상수 수정 - 화면 중앙 기준으로 위로 올라가도록 Y값을 음수로 설정
const FVector2D UComboCountWidget::COMBOCOUNT_IMAGE_POS = FVector2D(0.0f, -150.0f); // 중앙에서 위로 150픽셀
const FVector2D UComboCountWidget::COMBOCOUNT_TEXT_POS = FVector2D(0.0f, -30.0f); // 이미지 기준 상대 위치
const FLinearColor UComboCountWidget::COMBOCOUNT_TEXT_COLOR = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // 흰색

UComboCountWidget::UComboCountWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetAnimator = nullptr;
}

void UComboCountWidget::InitializeComboWidgets()
{
    UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
    if (!RootPanel)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 루트 패널을 찾을 수 없습니다."));
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget: 루트 패널 타입: %s"), *GetRootWidget()->GetClass()->GetName());

    // UIWidgetRenderer 인스턴스 가져오기 및 준비 확인
    UUIWidgetRenderer* WidgetRenderer = UUIWidgetRenderer::GetInstance();
    if (!WidgetRenderer)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: WidgetRenderer 인스턴스가 nullptr입니다."));
        return;
    }
    if (!WidgetRenderer->IsInViewport())
    {
        UE_LOG(LogTemp, Warning, TEXT("ComboCountWidget: WidgetRenderer가 아직 뷰포트에 없음. 재시도 대기."));
        // 아직 준비되지 않았다면 다음 틱에 다시 시도
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this]()
        {
            InitializeComboWidgets();
        }, 0.01f, false);
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget: WidgetRenderer 준비 완료."));

    // 이미지 위젯 생성 - UIWidgetRenderer 활용
    if (!ComboCountImage)
    {
        ComboCountImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (ComboCountImage)
        {
            WidgetRenderer->RenderUIImage(
                ComboCountImage,
                EWidgetAnchor::Center,
                TEXT("/Game/UI/PlayLevel/UI_Play_ComboCount"),
                FVector2D(353.0f * 1.5f, 78.0f * 1.5f),
                COMBOCOUNT_IMAGE_POS.X,
                COMBOCOUNT_IMAGE_POS.Y
            );

            RootPanel->AddChild(ComboCountImage);
            UE_LOG(LogTemp, Warning, TEXT("콤보 이미지 설정 완료 (UIWidgetRenderer 사용), ComboCountImage=%p"), ComboCountImage);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: ComboCountImage 생성 실패"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget: ComboCountImage 이미 존재, 주소=%p"), ComboCountImage);
    }

    // 텍스트 블록 생성
    if (!ComboCountTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: ComboCountTextBlock이 nullptr입니다. UMG 바인딩 확인 필요."));
    }
    else
    {
        UUIWidgetUtility::SetupTextBlockStyle(
            ComboCountTextBlock,
            COMBOCOUNT_TEXT_COLOR,
            56.0f,
            UUIWidgetUtility::DEFAULT_NUMBER_FONT_PATH,
            true,
            false,
            ESlateVisibility::HitTestInvisible
        );
        ComboCountTextBlock->SetText(FText::FromString(TEXT("0")));

        UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(RootPanel->AddChild(ComboCountTextBlock));
        if (TextSlot)
        {
            TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            FVector2D TextPos = COMBOCOUNT_IMAGE_POS + COMBOCOUNT_TEXT_POS;
            TextSlot->SetPosition(TextPos);
            TextSlot->SetSize(FVector2D(100.0f, 60.0f));
            TextSlot->SetZOrder(10);
            UE_LOG(LogTemp, Warning, TEXT("콤보 텍스트 위치: (%f,%f), ComboCountTextBlock=%p"), TextPos.X, TextPos.Y, ComboCountTextBlock);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: ComboCountTextBlock의 CanvasPanelSlot 생성 실패"));
        }
    }

    // 초기 숨김 상태
    if (WidgetAnimator && ComboCountImage && ComboCountTextBlock)
    {
        WidgetAnimator->SetComboCountVisibility(false);
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget: SetComboCountVisibility(false) 호출 완료"));
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 초기화 시 애니메이터 준비 안됨 (WidgetAnimator=%p, ComboCountImage=%p, ComboCountTextBlock=%p)"),
            WidgetAnimator, ComboCountImage, ComboCountTextBlock);
        if (ComboCountImage)
            ComboCountImage->SetVisibility(ESlateVisibility::Hidden);
        if (ComboCountTextBlock)
            ComboCountTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }

    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 초기화 완료"));
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

UComboCountWidget* UComboCountWidget::CreateComboCountWidget(UObject* WorldContextObject)
{
    // 이미 유효한 인스턴스가 있는지 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 이미 존재 - 기존 인스턴스 반환"));
        return Instance;
    }
    
    // 기존 인스턴스 정리
    if (Instance)
    {
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 인스턴스 발견됨 - 정리 후 재생성"));
        Instance = nullptr;
    }
    
    // 위젯 클래스 로드
    if (!LoadWidgetClassIfNeeded())
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget 클래스 로드 실패"));
        return nullptr;
    }
    
    APlayerController* Controller = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboCountWidget: 유효한 플레이어 컨트롤러가 없습니다"));
        return nullptr;
    }
    
    // 위젯 생성 및 뷰포트에 추가
    Instance = CreateWidget<UComboCountWidget>(Controller, ComboCountWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10002); // 점수 위젯보다 높은 Z-Order
        UE_LOG(LogTemp, Display, TEXT("ComboCountWidget 새로 생성 완료"));
    }
    
    return Instance;
}

void UComboCountWidget::BeginDestroy()
{
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    // 애니메이터 정리
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
        WidgetAnimator = nullptr;
    }
    
    Super::BeginDestroy();
}

bool UComboCountWidget::LoadWidgetClassIfNeeded()
{
    if (!ComboCountWidgetClass)
    {
        ComboCountWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_ComboCount.BP_UI_Play_ComboCount_C"));
    }
    return ComboCountWidgetClass != nullptr;
}

void UComboCountWidget::NativeConstruct()
{
    Super::NativeConstruct();

    Instance = this;

    // 1. 애니메이터 먼저 생성
    if (!WidgetAnimator)
    {
        WidgetAnimator = NewObject<UComboCountWidgetAnimator>(this, UComboCountWidgetAnimator::StaticClass());
    }

    // 2. 위젯 초기화
    InitializeComboWidgets();

    // 3. 애니메이터에 위젯 연결
    // ComboCountImage, ComboCountTextBlock은 아직 nullptr일 수 있으니, Initialize는 나중에!
    if (WidgetAnimator && ComboCountImage && ComboCountTextBlock)
    {
        WidgetAnimator->Initialize(ComboCountImage, ComboCountTextBlock);
        WidgetAnimator->SetComboCountVisibility(false); // 반드시 한 번 더 호출
    }

    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget NativeConstruct 완료"));
}

void UComboCountWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 애니메이터 정리
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
    }
    
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    UE_LOG(LogTemp, Display, TEXT("ComboCountWidget NativeDestruct 완료"));
}