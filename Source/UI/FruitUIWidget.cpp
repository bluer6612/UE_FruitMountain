#include "FruitUIWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetTree.h"

UFruitUIWidget::UFruitUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    WidgetPosition = EWidgetPosition::LeftTop;
    UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget 생성자 호출"));
}

void UFruitUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - NativeConstruct 호출"));
    
    // 자신의 주소 출력 (객체 추적)
    UE_LOG(LogTemp, Error, TEXT("FruitUIWidget 객체 주소: %p"), this);
    
    // 이미지 위젯이 없으면 생성
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - IconImage 없음, 생성 시작"));
        
        // 캔버스 패널 찾기 또는 생성
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
        if (!RootCanvas)
        {
            // 새 캔버스 생성
            RootCanvas = NewObject<UCanvasPanel>(this, TEXT("RootCanvas"));
            if (WidgetTree)
            {
                WidgetTree->RootWidget = RootCanvas;
                UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 새 루트 캔버스 생성"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - WidgetTree가 NULL임"));
            }
        }
        
        // 이미지 위젯 생성
        IconImage = NewObject<UImage>(this, TEXT("IconImage"));
        if (IconImage && RootCanvas)
        {
            // 이미지를 캔버스에 추가
            UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(IconImage);
            if (ImageSlot)
            {
                // 기본 색상으로 설정 (식별하기 쉽게 빨간색으로 변경)
                FSlateBrush RedBrush;
                RedBrush.DrawAs = ESlateBrushDrawType::Box;
                RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
                IconImage->SetBrush(RedBrush);
                
                // 위치와 크기 설정 (더 크게)
                ImageSlot->SetSize(FVector2D(300.0f, 300.0f));
                ImageSlot->SetPosition(FVector2D(100.0f, 100.0f));
                ImageSlot->SetZOrder(9999); // 최상위 Z-order
                
                UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 이미지 기본 설정 완료"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 이미지 슬롯 추가 실패"));
            }
        }
    }
    
    // 위젯 위치 업데이트
    UpdateWidgetPosition();
    
    // 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    if (IconImage)
    {
        IconImage->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 이미지 가시성 설정"));
    }
    
    UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 초기화 완료"));
}

void UFruitUIWidget::NativeDestruct()
{
    Super::NativeDestruct();
    UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 소멸"));
}

void UFruitUIWidget::SetWidgetPosition(EWidgetPosition NewPosition)
{
    WidgetPosition = NewPosition;
    UpdateWidgetPosition();
}

void UFruitUIWidget::SetImage(UTexture2D* NewTexture)
{
    UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - SetImage 호출"));
    
    // 이미지 컴포넌트 확인
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - SetImage 실패: IconImage가 NULL"));
        return;
    }
    
    // 항상 가시성 확보
    SetVisibility(ESlateVisibility::Visible);
    IconImage->SetVisibility(ESlateVisibility::Visible);
    
    if (NewTexture)
    {
        // 브러시 생성 및 설정
        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = FVector2D(300.0f, 300.0f);
        Brush.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        Brush.SetResourceObject(NewTexture);
        
        // 브러시 적용
        IconImage->SetBrush(Brush);
        IconImage->SetRenderOpacity(1.0f);
        IconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
        
        UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 이미지 설정 완료"));
        
        // 슬롯 설정 확인
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
        {
            CanvasSlot->SetSize(FVector2D(300.0f, 300.0f));
            CanvasSlot->SetZOrder(9999);
            UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 이미지 슬롯 크기 설정 완료"));
        }
        
        // 위젯 슬롯도 확인
        if (Slot)
        {
            if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(Slot))
            {
                ParentSlot->SetSize(FVector2D(300.0f, 300.0f));
                ParentSlot->SetZOrder(9999);
                UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 위젯 슬롯 설정 완료"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 빈 이미지 설정"));
        
        // 빈 브러시 설정
        FSlateBrush EmptyBrush;
        EmptyBrush.DrawAs = ESlateBrushDrawType::Box;
        EmptyBrush.TintColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f); // 회색 반투명
        IconImage->SetBrush(EmptyBrush);
    }
    
    // 위치 업데이트
    UpdateWidgetPosition();
}

void UFruitUIWidget::UpdateWidgetPosition()
{
    UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - UpdateWidgetPosition 호출"));
    
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - UpdateWidgetPosition 실패: IconImage가 NULL"));
        return;
    }
    
    // 화면 크기 구하기
    const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
    const float PaddingValue = EdgePadding;
    const FVector2D IconSize = FVector2D(300.0f, 300.0f);
    FVector2D Position;
    
    // 위치에 따라 좌표 계산
    switch (WidgetPosition)
    {
        case EWidgetPosition::LeftTop:
            Position = FVector2D(PaddingValue, PaddingValue);
            break;
        
        case EWidgetPosition::LeftMiddle:
            Position = FVector2D(PaddingValue, (ViewportSize.Y - IconSize.Y) / 2);
            break;
        
        case EWidgetPosition::RightTop:
            Position = FVector2D(ViewportSize.X - IconSize.X - PaddingValue, PaddingValue);
            break;
        
        default:
            Position = FVector2D(PaddingValue, PaddingValue);
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 위젯 위치 계산: X=%.1f, Y=%.1f"), Position.X, Position.Y);
    
    // 슬롯 설정
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
    {
        CanvasSlot->SetPosition(Position);
        CanvasSlot->SetSize(IconSize);
        CanvasSlot->SetZOrder(9999);
        UE_LOG(LogTemp, Warning, TEXT("FruitUIWidget - 이미지 슬롯 위치 설정 완료"));
    }
    
    // 자신과 아이콘 이미지 모두 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    IconImage->SetVisibility(ESlateVisibility::Visible);
}