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
}

void UFruitUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Warning, TEXT("NativeConstruct - 위젯 생성 시작"));
    
    // 이미지 위젯이 없으면 생성
    if (!IconImage)
    {
        UE_LOG(LogTemp, Warning, TEXT("IconImage 없음 - 직접 생성"));
        
        // 캔버스 패널 찾기 또는 생성
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
        if (!RootCanvas)
        {
            UE_LOG(LogTemp, Warning, TEXT("루트 캔버스 없음 - 새로 생성"));
            
            // 위젯 트리가 없으면 생성
            if (!WidgetTree)
            {
                UE_LOG(LogTemp, Error, TEXT("WidgetTree가 NULL - 위젯 생성 실패"));
                return;
            }
            
            // 새 캔버스 생성 후 루트 위젯으로 설정
            RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
            WidgetTree->RootWidget = RootCanvas;
            
            UE_LOG(LogTemp, Warning, TEXT("새 루트 캔버스 생성 및 설정 완료"));
        }
        
        // 이미지 위젯 생성
        IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (IconImage && RootCanvas)
        {
            // 이미지를 캔버스에 추가
            UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(IconImage);
            if (ImageSlot)
            {
                // 기본 빨간색으로 설정
                FSlateBrush RedBrush;
                RedBrush.DrawAs = ESlateBrushDrawType::Box;
                RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
                IconImage->SetBrush(RedBrush);
                
                // 위치와 크기 설정
                ImageSlot->SetSize(FVector2D(300.0f, 300.0f));
                ImageSlot->SetPosition(FVector2D(100.0f, 100.0f));
                ImageSlot->SetZOrder(999);
                
                UE_LOG(LogTemp, Warning, TEXT("이미지 위젯 생성 완료"));
            }
        }
    }
    
    // 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    if (IconImage)
    {
        IconImage->SetVisibility(ESlateVisibility::Visible);
    }
}

void UFruitUIWidget::SetImage(UTexture2D* NewTexture)
{
    UE_LOG(LogTemp, Warning, TEXT("SetImage 호출"));
    
    // 이미지 컴포넌트 확인
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("SetImage 실패: IconImage가 NULL"));
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
        Brush.ImageSize = FVector2D(300.0f, 300.0f); // 더 크게 설정
        Brush.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        Brush.SetResourceObject(NewTexture);
        
        // 브러시 적용
        IconImage->SetBrush(Brush);
        
        // 완전 불투명하게 설정
        IconImage->SetRenderOpacity(1.0f);
        IconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
        
        UE_LOG(LogTemp, Warning, TEXT("이미지 설정 완료: 크기 300x300"));
        
        // 슬롯 설정 확인
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
        {
            // 화면 중앙에 배치 (화면 크기에 관계없이)
            FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
            CanvasSlot->SetPosition(FVector2D(ViewportSize.X/2 - 150.0f, ViewportSize.Y/2 - 150.0f));
            CanvasSlot->SetSize(FVector2D(300.0f, 300.0f));
            CanvasSlot->SetZOrder(9999);
            
            UE_LOG(LogTemp, Warning, TEXT("이미지 슬롯 설정: 화면 중앙에 강제 배치"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("이미지 슬롯을 찾을 수 없음"));
        }
        
        // 강제로 재구성
        if (Slot)
        {
            UE_LOG(LogTemp, Warning, TEXT("위젯 슬롯 발견 - 위치 강제 조정"));
            
            // 위젯 자체의 슬롯 설정 (부모 위젯에서의 위치)
            if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(Slot))
            {
                FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
                ParentSlot->SetPosition(FVector2D(ViewportSize.X/2 - 150.0f, ViewportSize.Y/2 - 150.0f));
                ParentSlot->SetSize(FVector2D(300.0f, 300.0f));
                ParentSlot->SetZOrder(9999);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("빈 이미지 설정 - 빨간색 블록으로 표시"));
        
        // 이미지가 없으면 빨간색 블록으로 표시
        FSlateBrush EmptyBrush;
        EmptyBrush.DrawAs = ESlateBrushDrawType::Box;
        EmptyBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
        IconImage->SetBrush(EmptyBrush);
        
        // 슬롯 크기 설정
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
        {
            CanvasSlot->SetSize(FVector2D(300.0f, 300.0f));
        }
    }
}

void UFruitUIWidget::SetWidgetPosition(EWidgetPosition Position)
{
    WidgetPosition = Position;
    UpdateWidgetPosition();
}

void UFruitUIWidget::UpdateWidgetPosition()
{
    UE_LOG(LogTemp, Log, TEXT("UpdateWidgetPosition 호출"));
    
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateWidgetPosition 실패: IconImage가 NULL"));
        return;
    }
    
    // 화면 크기 구하기
    const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
    const float PaddingValue = EdgePadding;
    const FVector2D IconSize = FVector2D(256.0f, 256.0f);
    
    UE_LOG(LogTemp, Log, TEXT("화면 크기 정보 획득"));
    
    // 테스트용 - 모든 위젯을 화면 중앙에 배치
    FVector2D Position = FVector2D(ViewportSize.X/2 - IconSize.X/2, ViewportSize.Y/2 - IconSize.Y/2);
    
    UE_LOG(LogTemp, Log, TEXT("위젯 위치 계산 완료"));
    
    // 슬롯 설정
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
    {
        CanvasSlot->SetPosition(Position);
        CanvasSlot->SetSize(IconSize);
        CanvasSlot->SetZOrder(9999); // 최상위 표시
        CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        
        UE_LOG(LogTemp, Log, TEXT("이미지 슬롯 설정 완료"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 슬롯을 찾을 수 없음"));
    }
    
    // 자신과 아이콘 이미지 모두 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    IconImage->SetVisibility(ESlateVisibility::Visible);
    
    UE_LOG(LogTemp, Log, TEXT("위젯 가시성 설정 완료"));
}

// 빨간색 블록 표시 함수
void UFruitUIWidget::ShowRedBlock()
{
    UE_LOG(LogTemp, Warning, TEXT("빨간색 블록 표시 시도"));
    
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("IconImage가 NULL - 빨간색 블록 표시 실패"));
        return;
    }
    
    // 빨간색 블록 브러시 생성
    FSlateBrush RedBrush;
    RedBrush.DrawAs = ESlateBrushDrawType::Box;
    RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
    
    // 브러시 적용
    IconImage->SetBrush(RedBrush);
    IconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
    IconImage->SetRenderOpacity(1.0f);
    IconImage->SetVisibility(ESlateVisibility::Visible);
    
    // 위젯도 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    
    // 위치와 크기 설정
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
    {
        CanvasSlot->SetSize(FVector2D(300.0f, 300.0f));
        CanvasSlot->SetZOrder(9999);
        UE_LOG(LogTemp, Warning, TEXT("빨간색 블록 크기 및 Z-Order 설정 완료"));
    }
    
    // 위젯 자체의 슬롯 설정
    if (Slot)
    {
        if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(Slot))
        {
            FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
            ParentSlot->SetPosition(FVector2D(ViewportSize.X/2 - 150.0f, ViewportSize.Y/2 - 150.0f));
            ParentSlot->SetSize(FVector2D(300.0f, 300.0f));
            ParentSlot->SetZOrder(9999);
            UE_LOG(LogTemp, Warning, TEXT("위젯 슬롯 위치 설정 완료 - 화면 중앙"));
        }
    }
}