#include "UIWidgetRenderer.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "Engine/StreamableManager.h"

// 정적 인스턴스 초기화
UUIWidgetRenderer* UUIWidgetRenderer::Instance = nullptr;

UUIWidgetRenderer* UUIWidgetRenderer::CreateDisplayWidget(UObject* WorldContextObject)
{
    // 기존 인스턴스가 있으면 재사용
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }
    
    // 인스턴스 생성
    Instance = CreateWidget<UUIWidgetRenderer>(PC, UUIWidgetRenderer::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        Instance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    
    return Instance;
}

void UUIWidgetRenderer::NativeConstruct()
{
    Super::NativeConstruct();
    
    // static 플래그를 제거하고 항상 초기화하도록 수정
    Canvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!Canvas)
    {
        Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
        if (Canvas)
        {
            WidgetTree->RootWidget = Canvas;
            UE_LOG(LogTemp, Warning, TEXT("UIWidgetRenderer: 새 루트 캔버스 생성"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UIWidgetRenderer: 캔버스 생성 실패!"));
        }
    }
    
    // 인스턴스 설정
    if (IsValid(this))
    {
        Instance = this;
    }
}

// 위젯 소멸 시 정적 인스턴스 초기화
void UUIWidgetRenderer::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 이 위젯이 현재 싱글톤 인스턴스인 경우에만 초기화
    if (Instance == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("UIWidgetRenderer: 인스턴스 소멸, 정적 참조 초기화"));
        Instance = nullptr;
    }
}

bool UUIWidgetRenderer::IsInstanceValid()
{
    return Instance != nullptr && IsValid(Instance);
}

void UUIWidgetRenderer::SetupPlayImages()
{
    PrepareUIWidget(EWidgetImageType::UI_Play_Score, 
                   TEXT("/Game/UI/PlayLevel/UI_Play_Score"), 
                   FVector2D(504, 253), 40.0f, 30.0f);
                         
    PrepareUIWidget(EWidgetImageType::UI_Play_FruitList, 
                   TEXT("/Game/UI/PlayLevel/UI_Play_FruitList"), 
                   FVector2D(101, 762), 60.0f, 20.0f);
                         
    PrepareUIWidget(EWidgetImageType::UI_Play_NextFruit, 
                   TEXT("/Game/UI/PlayLevel/UI_Play_NextFruit"), 
                   FVector2D(301, 339), 120.0f, 60.0f);

    //UE_LOG(LogTemp, Warning, TEXT("UIWidgetRenderer: play 위젯 이미지 설정 완료"));
}

void UUIWidgetRenderer::SetupTitleImages()
{
    PrepareUIWidget(EWidgetImageType::UI_Title_Logo,
        TEXT("/Game/UI/TitleLevel/UI_Title_Logo"),
        FVector2D(633.f, 369.f), 80.f, 20.f);

    PrepareUIWidget(EWidgetImageType::UI_Title_Menu,
        TEXT("/Game/UI/TitleLevel/UI_Title_Menu"),
        FVector2D(592.f, 359.f), 80.f, 100.f);
}

void UUIWidgetRenderer::PrepareUIWidget(EWidgetImageType ImageType, const FString& TexturePath, const FVector2D& CustomSize, float PaddingX, float PaddingY)
{
    UImage** TargetImagePtr = nullptr;
    EWidgetAnchor Anchor = EWidgetAnchor::Center;

    switch (ImageType)
    {
        case EWidgetImageType::UI_Play_Score:
            TargetImagePtr = &UI_Play_Score;
            Anchor = EWidgetAnchor::TopLeft;
            break;
        case EWidgetImageType::UI_Play_FruitList:
            TargetImagePtr = &UI_Play_FruitList;
            Anchor = EWidgetAnchor::BottomLeft;
            break;
        case EWidgetImageType::UI_Play_NextFruit:
            TargetImagePtr = &UI_Play_NextFruit;
            Anchor = EWidgetAnchor::TopRight;
            break;
        case EWidgetImageType::UI_Title_Logo:
            TargetImagePtr = &UI_Title_Logo;
            Anchor = EWidgetAnchor::TopLeft;
            break;
        case EWidgetImageType::UI_Title_Menu:
            TargetImagePtr = &UI_Title_Menu;
            Anchor = EWidgetAnchor::BottomLeft;
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("알 수 없는 이미지 위치 유형: %d"), static_cast<int32>(ImageType));
            return;
    }

    if (!TargetImagePtr)
        return;

    RenderUIImage(*TargetImagePtr, Anchor, TexturePath, CustomSize, PaddingX, PaddingY);
}

// 직접 참조로 이미지 설정
void UUIWidgetRenderer::RenderUIImage(
    UImage*& ImageWidget,
    EWidgetAnchor Anchor,
    const FString& TexturePath,
    const FVector2D& CustomSize,
    float OffsetX,
    float OffsetY)
{
    if (!Canvas) return;

    if (ImageWidget)
    {
        ImageWidget->RemoveFromParent();
        ImageWidget = nullptr;
    }

    ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

    UTexture2D* LoededTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (LoededTexture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(LoededTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = CustomSize;
        ImageWidget->SetBrush(Brush);
        ImageWidget->SetColorAndOpacity(FLinearColor::White);
    }
    else
    {
        // 에러 브러시 처리
        FSlateBrush ErrorBrush;
        ErrorBrush.DrawAs = ESlateBrushDrawType::Box;
        ErrorBrush.TintColor = FLinearColor::Red;
        ImageWidget->SetBrush(ErrorBrush);
        ImageWidget->SetColorAndOpacity(FLinearColor::Red);
    }

    UCanvasPanelSlot* InSlot = Canvas->AddChildToCanvas(ImageWidget);
    if (InSlot)
    {
        UUIWidgetUtility::SetAnchorForSlot(InSlot, Anchor, OffsetX, OffsetY);
        InSlot->SetSize(CustomSize);
    }
}