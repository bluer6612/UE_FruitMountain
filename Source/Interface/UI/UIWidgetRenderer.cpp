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
    
    // NativeConstruct에서 직접 이미지 설정 호출
    SetupAllImages();
}

bool UUIWidgetRenderer::IsInstanceValid()
{
    return Instance != nullptr && IsValid(Instance);
}

void UUIWidgetRenderer::SetupAllImages()
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

    UE_LOG(LogTemp, Warning, TEXT("UIWidgetRenderer: 위젯 이미지 설정 완료"));
}

void UUIWidgetRenderer::PrepareUIWidget(EWidgetImageType ImageType, const FString& TexturePath, const FVector2D& CustomSize, float PaddingX, float PaddingY)
{
    // 이미지 참조와 앵커 정보 결정
    UImage** TargetImagePtr = nullptr;
    EWidgetAnchor Anchor = EWidgetAnchor::Center;
    
    // enum 값에 따라 이미지 참조와 앵커 설정
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
            
        default:
            UE_LOG(LogTemp, Error, TEXT("알 수 없는 이미지 위치 유형: %d"), static_cast<int32>(ImageType));
            return;
    }
    
    if (!TargetImagePtr) 
    {
        return;
    }
    
    // 내부 구현 함수 호출
    RenderUIImage(*TargetImagePtr, Anchor, TexturePath, CustomSize, PaddingX, PaddingY);
}

// 직접 참조로 이미지 설정
void UUIWidgetRenderer::RenderUIImage(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath, const FVector2D& CustomSize, float PaddingX, float PaddingY)
{
    // 캔버스 체크
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("UIWidgetRenderer: 캔버스가 없음!"));
        return;
    }
    
    // 기존 이미지가 있으면 제거
    if (ImageWidget)
    {
        ImageWidget->RemoveFromParent();
        ImageWidget = nullptr;
    }
    
    // 새 이미지 위젯 생성
    ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    if (!ImageWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("UIWidgetRenderer: 이미지 위젯 생성 실패!"));
        return;
    }
    
    // 캔버스에 추가
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(ImageWidget);
    if (!ImageSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 슬롯 생성 실패"));
        return;
    }
    
    // 텍스처 로드
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    
    if (LoadedTexture)
    {
        FVector2D FinalSize = CustomSize;
        if (FinalSize.X <= 0 || FinalSize.Y <= 0)
        {
            FinalSize.X = LoadedTexture->GetSizeX();
            FinalSize.Y = LoadedTexture->GetSizeY();
        }
        
        FSlateBrush Brush;
        Brush.SetResourceObject(LoadedTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = FinalSize;
        
        ImageWidget->SetBrush(Brush);
        ImageWidget->SetColorAndOpacity(FLinearColor::White);
        
        ImageSlot->SetSize(FinalSize);
        UUIWidgetUtility::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Display, TEXT("이미지 위젯 설정 완료: %s"), *TexturePath);
    }
    else
    {
        // 로드 실패 시 빨간색 박스와 텍스트 추가
        FSlateBrush ErrorBrush;
        ErrorBrush.DrawAs = ESlateBrushDrawType::Box;
        ErrorBrush.TintColor = FLinearColor::Red;
        
        ImageWidget->SetBrush(ErrorBrush);
        ImageWidget->SetColorAndOpacity(FLinearColor::Red);
        
        // 로드 실패 텍스트 추가
        UTextBlock* ErrorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (ErrorText)
        {
            Canvas->AddChild(ErrorText);
            
            // 경로에서 파일명만 추출
            FString FileName = FPaths::GetCleanFilename(TexturePath);
            ErrorText->SetText(FText::FromString(FString::Printf(TEXT("로드 실패:\n%s"), *FileName)));
            
            // 텍스트 스타일 설정
            ErrorText->SetColorAndOpacity(FLinearColor::White);
            
            // 텍스트를 이미지와 같은 위치에 배치
            UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(ErrorText->Slot);
            if (TextSlot)
            {
                TextSlot->SetSize(CustomSize);
                UUIWidgetUtility::SetAnchorForSlot(TextSlot, Anchor, PaddingX, PaddingY);
                TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            }
        }
        
        ImageSlot->SetSize(CustomSize);
        UUIWidgetUtility::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *TexturePath);
    }
}