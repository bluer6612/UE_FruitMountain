#include "TextureDisplayWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

// 정적 인스턴스 초기화
UTextureDisplayWidget* UTextureDisplayWidget::Instance = nullptr;

UTextureDisplayWidget* UTextureDisplayWidget::CreateDisplayWidget(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    // 이미 인스턴스가 있으면 반환
    if (IsValid(Instance))
    {
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 기존 인스턴스 재사용"));
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10000);
        }
        return Instance;
    }

    // 새 인스턴스 생성
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return nullptr;
    
    Instance = CreateWidget<UTextureDisplayWidget>(PC, UTextureDisplayWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 새 인스턴스 생성 성공"));
    }
    
    return Instance;
}

void UTextureDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 루트 캔버스 가져오기
    Canvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!Canvas)
    {
        Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
        WidgetTree->RootWidget = Canvas;
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 새 루트 캔버스 생성"));
    }
    
    // 정적 참조 업데이트
    Instance = this;
    
    // 이미지 설정
    SetupAllImages();
}

void UTextureDisplayWidget::SetupAllImages()
{
    UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 이미지 설정 시작"));
    
    // 화면 크기를 고려한 앵커 기반 위치 설정
    SetupImageWithTexture(LeftTopImage, EWidgetAnchor::TopLeft, TEXT("/Game/Asset/UI/UI_Play_Score"));
    SetupImageWithTexture(LeftMiddleImage, EWidgetAnchor::MiddleLeft, TEXT("/Game/Asset/UI/UI_Play_FruitList"));
    SetupImageWithTexture(RightTopImage, EWidgetAnchor::TopRight, TEXT("/Game/Asset/UI/UI_Play_NextFruit"));

    UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 이미지 설정 완료"));
}

UImage* UTextureDisplayWidget::GetImageForType(EWidgetImageType Type)
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

void UTextureDisplayWidget::SetImageTexture(EWidgetImageType Position, const FString& TexturePath)
{
    UImage* TargetImage = GetImageForType(Position);
    
    // 앵커 타입 매핑
    EWidgetAnchor Anchor;
    switch (Position)
    {
        case EWidgetImageType::LeftTop:
            Anchor = EWidgetAnchor::TopLeft;
            break;
        case EWidgetImageType::LeftMiddle:
            Anchor = EWidgetAnchor::MiddleLeft;
            break;
        case EWidgetImageType::RightTop:
            Anchor = EWidgetAnchor::TopRight;
            break;
        default:
            Anchor = EWidgetAnchor::Center;
            break;
    }
    
    // 새 함수 호출
    SetupImageWithTexture(TargetImage, Anchor, TexturePath);
}

// 앵커 기반 이미지 설정 함수
void UTextureDisplayWidget::SetupImageWithTexture(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath)
{
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("TextureDisplayWidget: 캔버스가 없음!"));
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
        UE_LOG(LogTemp, Error, TEXT("TextureDisplayWidget: 이미지 위젯 생성 실패!"));
        return;
    }
    
    // 캔버스에 추가
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(ImageWidget);
    if (!ImageSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("TextureDisplayWidget: 이미지 슬롯 생성 실패!"));
        return;
    }
    
    // 텍스처 로드 및 적용 (UIHelper 클래스 사용)
    UTexture2D* LoadedTexture = UUIHelper::LoadAndApplyTexture(ImageWidget, TexturePath);
    
    if (LoadedTexture)
    {
        // 실제 텍스처 크기를 슬롯에 적용
        ImageSlot->SetSize(FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY()));
        
        // 앵커 기반 위치 설정 (UIHelper 사용)
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor);
    }
    else
    {
        // 텍스처 로드 실패 시 기본 크기 설정
        ImageSlot->SetSize(FVector2D(200.0f, 200.0f));
        
        // 앵커 기반 위치 설정 (에러 표시용)
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor);
    }
}