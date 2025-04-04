// SimpleTextureWidget.cpp
#include "SimpleTextureWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

// 정적 인스턴스 초기화
USimpleTextureWidget* USimpleTextureWidget::Instance = nullptr;

USimpleTextureWidget* USimpleTextureWidget::CreateSimpleUI(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    // 현재 레벨 확인
    FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(World);
    if (CurrentLevel != TEXT("PlayLevel"))
    {
        UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 현재 레벨(%s)은 PlayLevel이 아님"), *CurrentLevel);
        return nullptr;
    }

    // 새 인스턴스 생성
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;
    
    Instance = CreateWidget<USimpleTextureWidget>(PC, USimpleTextureWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        
        // 컨트롤러 설정 추가
        if (PC)
        {
            PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(Instance->TakeWidget()));
            PC->SetShowMouseCursor(true); // 마우스 커서 표시 (UI 작업할 때 유용)
            
            UE_LOG(LogTemp, Warning, TEXT("PlayerController 입력 모드가 GameAndUI로 설정됨"));
        }
        
        // 명시적 가시성 설정
        Instance->SetVisibility(ESlateVisibility::Visible);
        
        // 디버깅 로그 추가
        UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 화면에 추가됨 - Z-Order: 10000"));
        
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
    }
    
    // 정적 참조 업데이트
    Instance = this;
    
    // 모든 이미지 설정
    SetupAllImages();
}

void USimpleTextureWidget::SetupAllImages()
{
    UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 이미지 설정 시작"));
    
    // 각 위치에 이미지 설정
    SetupImage(LeftTopImage, FVector2D(50.0f, 50.0f), TEXT("UI_Play_Score"));
    SetupImage(LeftMiddleImage, FVector2D(50.0f, 300.0f), TEXT("UI_Play_FruitList"));
    SetupImage(RightTopImage, FVector2D(800.0f, 50.0f), TEXT("UI_Play_NextFruit"));
    
    UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: 이미지 설정 완료"));
}

void USimpleTextureWidget::SetupImage(UImage*& ImagePtr, const FVector2D& Position, const FString& TextureName)
{
    UCanvasPanel* Canvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!Canvas) return;
    
    // 이미 존재하면 삭제
    if (ImagePtr)
    {
        ImagePtr->RemoveFromParent();
        ImagePtr = nullptr;
    }
    
    // 새 이미지 생성
    ImagePtr = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    if (!ImagePtr) return;
    
    // 캔버스에 추가
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(ImagePtr);
    if (ImageSlot)
    {
        // 위치/크기 설정
        ImageSlot->SetPosition(Position);
        ImageSlot->SetSize(FVector2D(300.0f, 300.0f)); // 더 크게 설정
        
        // 텍스처 로드
        FString TexturePath = FString::Printf(TEXT("/Game/Asset/UI/%s.%s"), *TextureName, *TextureName);
        UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
        
        if (LoadedTexture)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(LoadedTexture);
            Brush.DrawAs = ESlateBrushDrawType::Image;
            Brush.ImageSize = FVector2D(300.0f, 300.0f); // 더 크게 설정
            
            ImagePtr->SetBrush(Brush);
            ImagePtr->SetVisibility(ESlateVisibility::Visible);
            
            UE_LOG(LogTemp, Warning, TEXT("SimpleTextureWidget: %s 텍스처 로드 성공"), *TextureName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SimpleTextureWidget: %s 텍스처 로드 실패 (경로: %s)"), *TextureName, *TexturePath);
            
            // 텍스처 로드 실패 시 빨간색 표시
            FSlateBrush DefaultBrush;
            DefaultBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
            DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
            ImagePtr->SetBrush(DefaultBrush);
        }
    }
}