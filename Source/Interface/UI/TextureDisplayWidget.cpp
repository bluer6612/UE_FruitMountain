#include "TextureDisplayWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"

// 정적 인스턴스 초기화
UTextureDisplayWidget* UTextureDisplayWidget::Instance = nullptr;

UTextureDisplayWidget* UTextureDisplayWidget::CreateDisplayWidget(UObject* WorldContextObject)
{
    // 기존 인스턴스가 있으면 재사용
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 가져오기
    APlayerController* PC = UUIHelper::GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }
    
    // 인스턴스 생성
    Instance = CreateWidget<UTextureDisplayWidget>(PC, UTextureDisplayWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        Instance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    
    return Instance;
}

// 위젯 소멸 시 정적 인스턴스 초기화
void UTextureDisplayWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 이 위젯이 현재 싱글톤 인스턴스인 경우에만 초기화
    if (Instance == this)
    {
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 인스턴스 소멸, 정적 참조 초기화"));
        Instance = nullptr;
    }
}

void UTextureDisplayWidget::NativeConstruct()
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
            UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 새 루트 캔버스 생성"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("TextureDisplayWidget: 캔버스 생성 실패!"));
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

bool UTextureDisplayWidget::IsInstanceValid()
{
    return Instance != nullptr && IsValid(Instance);
}

void UTextureDisplayWidget::SetupAllImages()
{
    // 화면 크기를 고려한 앵커 기반 위치 설정 + 개별 패딩값 적용
    SetupImage(EWidgetImageType::UI_Play_Score, TEXT("/Game/UI/PlayLevel/UI_Play_Score"), 
               FVector2D(504, 253), 40.0f, 30.0f); // 왼쪽 상단 점수판
                         
    SetupImage(EWidgetImageType::UI_Play_FruitList, TEXT("/Game/UI/PlayLevel/UI_Play_FruitList"), 
               FVector2D(101, 762), 60.0f, 20.0f); // 왼쪽 하단 과일 목록
                         
    SetupImage(EWidgetImageType::UI_Play_NextFruit, TEXT("/Game/UI/PlayLevel/UI_Play_NextFruit"), 
               FVector2D(301, 339), 120.0f, 60.0f); // 오른쪽 상단 다음 과일

    UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 위젯 이미지 설정 완료"));
}

// 통합된 이미지 설정 함수: enum 또는 직접 참조 모두 처리 가능
void UTextureDisplayWidget::SetupImage(EWidgetImageType ImageType, const FString& TexturePath, 
                                     const FVector2D& CustomSize, 
                                     float PaddingX, float PaddingY)
{
    // 이미지 참조와 앵커 정보를 한 번에 결정
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
    
    // 유효한 이미지 참조가 있는지 확인
    if (!TargetImagePtr || !Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 참조가 유효하지 않거나 캔버스가 없음"));
        return;
    }
    
    // 기존 이미지가 있으면 제거
    if (*TargetImagePtr)
    {
        (*TargetImagePtr)->RemoveFromParent();
        *TargetImagePtr = nullptr;
    }
    
    // 새 이미지 생성
    *TargetImagePtr = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    if (!(*TargetImagePtr))
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 위젯 생성 실패"));
        return;
    }
    
    // 캔버스에 추가
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(*TargetImagePtr);
    if (!ImageSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 슬롯 생성 실패"));
        return;
    }
    
    // 텍스처 로드 및 적용
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (!LoadedTexture)
    {
        LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
    }
    if (!LoadedTexture)
    {
        FStreamableManager StreamableManager;
        FSoftObjectPath AssetRef(TexturePath);
        LoadedTexture = Cast<UTexture2D>(StreamableManager.LoadSynchronous(AssetRef));
    }
    
    if (LoadedTexture)
    {
        // 사용자 지정 크기 또는 원본 크기 사용
        FVector2D FinalSize = CustomSize;
        if (FinalSize.X <= 0 || FinalSize.Y <= 0)
        {
            FinalSize.X = LoadedTexture->GetSizeX();
            FinalSize.Y = LoadedTexture->GetSizeY();
        }
        
        // 브러시 설정
        FSlateBrush Brush;
        Brush.SetResourceObject(LoadedTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = FinalSize;
        
        // 이미지에 브러시 적용
        (*TargetImagePtr)->SetBrush(Brush);
        (*TargetImagePtr)->SetColorAndOpacity(FLinearColor::White);
        
        // 슬롯 크기 설정
        ImageSlot->SetSize(FinalSize);
        
        // 앵커 설정
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Display, TEXT("이미지 위젯 설정 완료: %s"), *TexturePath);
    }
    else
    {
        // 로드 실패 시 빨간색 박스 표시
        FSlateBrush ErrorBrush;
        ErrorBrush.DrawAs = ESlateBrushDrawType::Box;
        ErrorBrush.TintColor = FLinearColor::Red;
        
        (*TargetImagePtr)->SetBrush(ErrorBrush);
        (*TargetImagePtr)->SetColorAndOpacity(FLinearColor::Red);
        
        // 슬롯 크기와 위치는 설정
        ImageSlot->SetSize(CustomSize);
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *TexturePath);
    }
}

// 직접 UImage 참조를 위한 오버로드 (필요한 경우)
void UTextureDisplayWidget::SetupImage(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath,
                                     const FVector2D& CustomSize, 
                                     float PaddingX, float PaddingY)
{
    // 캔버스 체크
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("TextureDisplayWidget: 캔버스가 없음!"));
        return;
    }
    
    // 기존 이미지가 있으면 제거
    if (ImageWidget)
    {
        ImageWidget->RemoveFromParent();
        ImageWidget = nullptr;
    }
    
    // 나머지 코드는 위와 동일하게 구현
    UCanvasPanelSlot* ImageSlot = Canvas->AddChildToCanvas(ImageWidget);
    if (!ImageSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 슬롯 생성 실패"));
        return;
    }
    
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (!LoadedTexture)
    {
        LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
    }
    if (!LoadedTexture)
    {
        FStreamableManager StreamableManager;
        FSoftObjectPath AssetRef(TexturePath);
        LoadedTexture = Cast<UTexture2D>(StreamableManager.LoadSynchronous(AssetRef));
    }
    
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
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Display, TEXT("이미지 위젯 설정 완료: %s"), *TexturePath);
    }
    else
    {
        FSlateBrush ErrorBrush;
        ErrorBrush.DrawAs = ESlateBrushDrawType::Box;
        ErrorBrush.TintColor = FLinearColor::Red;
        
        ImageWidget->SetBrush(ErrorBrush);
        ImageWidget->SetColorAndOpacity(FLinearColor::Red);
        
        ImageSlot->SetSize(CustomSize);
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
        
        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *TexturePath);
    }
}