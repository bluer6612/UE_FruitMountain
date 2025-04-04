#include "SimpleTextureWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

// 정적 인스턴스 초기화
USimpleTextureWidget* USimpleTextureWidget::Instance = nullptr;

USimpleTextureWidget* USimpleTextureWidget::CreateSimpleUI(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    // 이미 인스턴스가 있으면 반환
    if (IsValid(Instance))
    {
        UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 기존 인스턴스 재사용"));
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10000);
        }
        return Instance;
    }

    // 새 인스턴스 생성
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;
    
    Instance = CreateWidget<USimpleTextureWidget>(PC, USimpleTextureWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 새 인스턴스 생성 성공"));
    }
    
    return Instance;
}

void USimpleTextureWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 루트 캔버스 확인/생성
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!RootCanvas)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
        WidgetTree->RootWidget = RootCanvas;
        UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 새 루트 캔버스 생성"));
    }
    
    // 정적 참조 업데이트
    Instance = this;
    
    // 이미지 설정
    SetupAllImages();
}

void USimpleTextureWidget::SetupAllImages()
{
    UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 이미지 설정 시작"));
    
    // 각 위치에 이미지 설정// 엔진 기본 텍스처 사용 (테스트용)
    SetupImageWithTexture(LeftTopImage, FVector2D(50.0f, 50.0f), TEXT("/Game/Asset/UI/UI_Play_Score"));
    SetupImageWithTexture(LeftMiddleImage, FVector2D(50.0f, 300.0f), TEXT("/Game/Asset/UI/UI_Play_FruitList"));
    SetupImageWithTexture(RightTopImage, FVector2D(800.0f, 50.0f), TEXT("/Game/Asset/UI/UI_Play_NextFruit"));
    
    // 엔진 내장 텍스처도 추가 (비교용)
    SetupImageWithTexture(nullptr, FVector2D(400.0f, 400.0f), TEXT("/Engine/EditorResources/S_Actor"));

    UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 이미지 설정 완료"));
}

UImage* USimpleTextureWidget::GetImageForType(EWidgetImageType Type)
{
    switch (Type)
    {
        case EWidgetImageType::LeftTop:
            return LeftTopImage;
        case EWidgetImageType::LeftMiddle:
            return LeftMiddleImage;
        case EWidgetImageType::RightTop:
            return RightTopImage;
        default:
            return nullptr;
    }
}

void USimpleTextureWidget::SetImageTexture(EWidgetImageType Position, const FString& TexturePath)
{
    UImage* TargetImage = GetImageForType(Position);
    
    FVector2D ImagePosition;
    switch (Position)
    {
        case EWidgetImageType::LeftTop:
            ImagePosition = FVector2D(50.0f, 50.0f);
            break;
        case EWidgetImageType::LeftMiddle:
            ImagePosition = FVector2D(50.0f, 300.0f);
            break;
        case EWidgetImageType::RightTop:
            ImagePosition = FVector2D(800.0f, 50.0f);
            break;
    }
    
    SetupImageWithTexture(TargetImage, ImagePosition, TexturePath);
}

void USimpleTextureWidget::SetupImageWithTexture(UImage* ImageWidget, const FVector2D& Position, const FString& TexturePath)
{
    UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("SimpleTextureWidget: 캔버스가 없음!"));
        return;
    }
    
    // 이미 존재하면 삭제
    if (ImageWidget)
    {
        ImageWidget->RemoveFromParent();
        ImageWidget = nullptr;
    }
    
    // 새 이미지 생성
    ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    if (!ImageWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("SimpleTextureWidget: 이미지 위젯 생성 실패!"));
        return;
    }
    
    // 캔버스에 추가
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(ImageWidget);
    if (!ImageSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("SimpleTextureWidget: 이미지 슬롯 생성 실패!"));
        return;
    }
    
    // 위치/크기 설정
    ImageSlot->SetPosition(Position);
    ImageSlot->SetSize(FVector2D(300.0f, 300.0f)); // 크게 설정
    
    // 텍스처 로드 (TSoftObjectPtr 및 StaticLoadObject 사용)
    UTexture2D* LoadedTexture = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 시도: %s"), *TexturePath);

    LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (LoadedTexture)
    {
        UE_LOG(LogTemp, Warning, TEXT("========================================="));
        UE_LOG(LogTemp, Warning, TEXT("텍스처 정보:"));
        UE_LOG(LogTemp, Warning, TEXT("- 경로: %s"), *TexturePath);
        UE_LOG(LogTemp, Warning, TEXT("- 크기: %dx%d"), LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());
        UE_LOG(LogTemp, Warning, TEXT("- 필터 방식: %d"), static_cast<int32>(LoadedTexture->Filter));
        UE_LOG(LogTemp, Warning, TEXT("- LOD 바이어스: %d"), LoadedTexture->LODBias);
        UE_LOG(LogTemp, Warning, TEXT("- 압축 설정: %d"), static_cast<int32>(LoadedTexture->CompressionSettings));
        UE_LOG(LogTemp, Warning, TEXT("========================================="));
    }
    else
    {
        // 실패 시 StaticLoadObject 시도
        LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
    }

    if (LoadedTexture)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(LoadedTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        
        // 원본 텍스처 크기 사용 (고정 크기 대신)
        int32 TexWidth = LoadedTexture->GetSizeX();
        int32 TexHeight = LoadedTexture->GetSizeY();
        
        // 너무 작으면 최소 크기 적용
        if (TexWidth < 64 || TexHeight < 64)
        {
            UE_LOG(LogTemp, Warning, TEXT("텍스처 크기가 너무 작음 (%dx%d), 크기를 조정합니다"), TexWidth, TexHeight);
            TexWidth = FMath::Max(TexWidth, 300);
            TexHeight = FMath::Max(TexHeight, 300);
        }
        
        Brush.ImageSize = FVector2D(TexWidth, TexHeight);
        ImageSlot->SetSize(FVector2D(TexWidth, TexHeight));
        
        // 브러시 크기와 슬롯 크기를 명시적으로 큰 값으로 설정
        const int32 ForcedSize = 300; // 강제 크기
        Brush.ImageSize = FVector2D(ForcedSize, ForcedSize);
        ImageSlot->SetSize(FVector2D(ForcedSize, ForcedSize));

        // 로그에 강제 크기 표시
        UE_LOG(LogTemp, Warning, TEXT("강제 크기 적용: %dx%d"), ForcedSize, ForcedSize);
        
        // 로그에 원본 크기와 조정된 크기 표시
        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 성공! 경로: %s, 원본 크기: %dx%d, 적용 크기: %dx%d"), 
            *TexturePath, LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY(), TexWidth, TexHeight);
        
        ImageWidget->SetBrush(Brush);
        ImageWidget->SetColorAndOpacity(FLinearColor::White); // 밝게 설정
        ImageWidget->SetRenderOpacity(1.0f); // 완전히 불투명하게
        ImageWidget->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        // 텍스처 로드 실패 시 빨간색 박스 표시
        FSlateBrush DefaultBrush;
        DefaultBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 밝은 빨간색
        DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
        
        ImageWidget->SetBrush(DefaultBrush);
        ImageWidget->SetVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogTemp, Error, TEXT("SimpleTextureWidget: 텍스처 로드 실패! 경로: %s"), *TexturePath);
    }
    
    // 강제 시각화 설정
    ImageSlot->SetAutoSize(false);
    ImageSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    ImageWidget->SetDesiredSizeOverride(FVector2D(300.0f, 300.0f));
}