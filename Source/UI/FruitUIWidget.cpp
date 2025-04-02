#include "FruitUIWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UFruitUIWidget::UFruitUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetPosition = EWidgetPosition::LeftTop;
}

void UFruitUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // C++에서 위젯 트리 구성 - 디자인 타임이 아닐 때만 실행
    if (!IsDesignTime() && GetWidgetTree())
    {
        // 루트 캔버스 패널 생성
        UCanvasPanel* RootCanvas = GetWidgetTree()->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
        if (RootCanvas)
        {
            // 루트 위젯으로 설정
            GetWidgetTree()->RootWidget = RootCanvas;
            
            // 이미지 위젯 생성 - 런타임에 동적으로 생성
            IconImage = GetWidgetTree()->ConstructWidget<UImage>(UImage::StaticClass());
            if (IconImage)
            {
                // 이미지를 캔버스에 추가
                UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(IconImage);
                if (ImageSlot)
                {
                    // 슬롯 설정 - 기본 이미지가 없어도 슬롯 먼저 설정
                    ImageSlot->SetAnchors(FAnchors(0, 0, 0, 0));
                    ImageSlot->SetAlignment(FVector2D(0, 0));
                    ImageSlot->SetSize(FVector2D(128.0f, 128.0f));
                    
                    // 빈 이미지로 초기화 - 나중에 SetImage()로 변경 가능
                    FSlateBrush EmptyBrush;
                    EmptyBrush.DrawAs = ESlateBrushDrawType::Image;
                    EmptyBrush.ImageSize = FVector2D(128.0f, 128.0f);
                    // 텍스처는 설정하지 않음 - SetImage()에서 설정
                    IconImage->SetBrush(EmptyBrush);
                    
                    UE_LOG(LogTemp, Log, TEXT("UI 위젯 초기화 완료 - 슬롯 설정됨, 이미지는 비어있음"));
                }
            }
        }
    }
    
    // 위치 업데이트 - 위젯 생성 여부와 관계없이 항상 호출
    UpdateWidgetPosition();
}

void UFruitUIWidget::SetImage(UTexture2D* NewTexture)
{
    // 이미지 컴포넌트 확인
    if (!IconImage)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetImage 실패: 이미지 컴포넌트가 없습니다"));
        return;
    }
    
    // 텍스처가 없으면 투명 이미지로 설정
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::Image;
    Brush.ImageSize = FVector2D(128.0f, 128.0f);
    
    if (NewTexture)
    {
        // 실제 텍스처 설정
        Brush.SetResourceObject(NewTexture);
        UE_LOG(LogTemp, Verbose, TEXT("이미지 설정: %s"), *NewTexture->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("빈 이미지 설정"));
    }
    
    // 브러시 적용
    IconImage->SetBrush(Brush);
}

void UFruitUIWidget::SetWidgetPosition(EWidgetPosition Position)
{
    WidgetPosition = Position;
    UpdateWidgetPosition();
}

void UFruitUIWidget::UpdateWidgetPosition()
{
    if (!IconImage)
        return;
    
    // 화면 크기 구하기
    const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
    // 지역 변수명 변경하여 멤버 변수와 충돌 방지
    const float PaddingValue = EdgePadding;
    const FVector2D IconSize = FVector2D(128.0f, 128.0f);
    
    // 위치 계산
    FVector2D Position;
    
    switch (WidgetPosition)
    {
        case EWidgetPosition::LeftTop:
            Position = FVector2D(PaddingValue, PaddingValue);
            break;
            
        case EWidgetPosition::LeftMiddle:
            Position = FVector2D(PaddingValue, (ViewportSize.Y - IconSize.Y) / 2.0f);
            break;
            
        case EWidgetPosition::RightTop:
            Position = FVector2D(ViewportSize.X - IconSize.X - PaddingValue, PaddingValue);
            break;
            
        default:
            Position = FVector2D(0, 0);
            break;
    }
    
    // 캔버스 슬롯 가져와서 위치 설정
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
    {
        CanvasSlot->SetPosition(Position);
        CanvasSlot->SetSize(IconSize);
    }
}