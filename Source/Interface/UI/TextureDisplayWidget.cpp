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
    
    // 인스턴스 유효성 검사 강화 - IsValid 함수만 사용하여 단순화
    if (Instance && IsValid(Instance) && Instance->GetIsVisible())
    {
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 유효한 기존 인스턴스 재사용"));
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10000);
        }
        return Instance;
    }
    else
    {
        // 기존 인스턴스가 무효하면 null로 설정
        if (Instance)
        {
            UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 무효한 인스턴스 폐기"));
            Instance = nullptr;
        }
    }

    // 새 인스턴스 생성
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller) return nullptr;
    
    Instance = CreateWidget<UTextureDisplayWidget>(Controller, UTextureDisplayWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10000);
        UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 새 인스턴스 생성 성공"));
    }
    
    return Instance;
}

// 추가: 위젯 소멸 시 정적 인스턴스 초기화
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

// NativeConstruct도 안전하게 업데이트
void UTextureDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    static bool bAlreadyConstructed = false;
    
    // 루트 캔버스 가져오기 (null 체크 추가)
    if (WidgetTree and bAlreadyConstructed)
    {
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
    }
    else
    {
        bAlreadyConstructed = true;
    }
    
    // 인스턴스가 유효하면 정적 참조 업데이트
    if (IsValid(this))
    {
        Instance = this;
    }
}

void UTextureDisplayWidget::SetupAllImages()
{
    // 화면 크기를 고려한 앵커 기반 위치 설정 + 개별 패딩값 적용
    SetupImageWithTexture(UI_Play_Score, EWidgetAnchor::TopLeft, 
                         TEXT("/Game/Asset/UI/UI_Play_Score"), 
                         FVector2D(504, 253),
                         40.0f, 30.0f); // 왼쪽 상단 점수판
                         
    SetupImageWithTexture(UI_Play_FruitList, EWidgetAnchor::BottomLeft, 
                         TEXT("/Game/Asset/UI/UI_Play_FruitList"), 
                         FVector2D(101, 762),
                         60.0f, 20.0f); // 왼쪽 하단 과일 목록
                         
    SetupImageWithTexture(UI_Play_NextFruit, EWidgetAnchor::TopRight, 
                         TEXT("/Game/Asset/UI/UI_Play_NextFruit"), 
                         FVector2D(301, 339),
                         120.0f, 60.0f); // 오른쪽 상단 다음 과일

    UE_LOG(LogTemp, Warning, TEXT("TextureDisplayWidget: 위젯 이미지 설정 완료"));
}

void UTextureDisplayWidget::SetImageTexture(EWidgetImageType Position, const FString& TexturePath, const FVector2D& CustomSize, float PaddingX, float PaddingY)
{
    // 이미지 참조와 앵커 정보를 한 번에 결정
    UImage** TargetImagePtr = nullptr;
    EWidgetAnchor Anchor = EWidgetAnchor::Center;
    
    // 단일 switch 문으로 해당 위치의 이미지 참조와 앵커 타입을 함께 결정
    switch (Position)
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
            UE_LOG(LogTemp, Error, TEXT("알 수 없는 이미지 위치 유형: %d"), static_cast<int32>(Position));
            return;
    }
    
    // 유효한 이미지 참조가 있는지 확인
    if (!TargetImagePtr)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 참조를 찾을 수 없음"));
        return;
    }
    
    // 이미지 설정 함수 호출 (참조로 전달)
    SetupImageWithTexture(*TargetImagePtr, Anchor, TexturePath, CustomSize, PaddingX, PaddingY);
}

// 앵커 기반 이미지 설정 함수 - 패딩 매개변수 추가
void UTextureDisplayWidget::SetupImageWithTexture(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath, const FVector2D& CustomSize, float PaddingX, float PaddingY)
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
        // 사용자 지정 크기 또는 원본 크기 사용
        FVector2D FinalSize = CustomSize;
        
        // FSlateBrush에도 사용자 지정 크기 적용 (UIHelper 수정 필요)
        FSlateBrush Brush;
        Brush.SetResourceObject(LoadedTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = FinalSize;
        ImageWidget->SetBrush(Brush);
        
        // 슬롯 크기 설정
        ImageSlot->SetSize(FinalSize);
        
        // 앵커 기반 위치 설정 (패딩 값 전달)
        UUIHelper::SetAnchorForSlot(ImageSlot, Anchor, PaddingX, PaddingY);
    }
    else
    {
        // 패딩 값 로그 출력
        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패, 패딩 적용: X=%.1f, Y=%.1f"), PaddingX, PaddingY);
    }
}