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
    
    // 이미지 컴포넌트가 없으면 생성
    if (!IconImage)
    {
        IconImage = NewObject<UImage>(this, TEXT("IconImage"));
        if (IconImage)
        {
            // Canvas Panel에 추가
            UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
            if (RootCanvas)
            {
                RootCanvas->AddChild(IconImage);
            }
        }
    }
    
    // 초기 위치 설정
    UpdateWidgetPosition();
}

void UFruitUIWidget::SetImage(UTexture2D* NewTexture)
{
    if (IconImage && NewTexture)
    {
        // 이미지 설정
        FSlateBrush Brush;
        Brush.SetResourceObject(NewTexture);
        Brush.ImageSize = FVector2D(128.0f, 128.0f); // 기본 크기 설정
        
        IconImage->SetBrush(Brush);
    }
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
    const float Padding = 50.0f;
    const FVector2D IconSize = FVector2D(128.0f, 128.0f);
    
    // 위치 계산
    FVector2D Position;
    
    switch (WidgetPosition)
    {
        case EWidgetPosition::LeftTop:
            Position = FVector2D(Padding, Padding);
            break;
            
        case EWidgetPosition::LeftMiddle:
            Position = FVector2D(Padding, (ViewportSize.Y - IconSize.Y) / 2.0f);
            break;
            
        case EWidgetPosition::RightTop:
            Position = FVector2D(ViewportSize.X - IconSize.X - Padding, Padding);
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