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
    UpdateWidgetPosition();  // 실제 화면 위치 갱신
}

void UFruitUIWidget::SetImage(UTexture2D* NewTexture)
{
    UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - SetImage 호출 [%p]"), NewTexture);
    
    // IconImage 컴포넌트 확인
    if (!IconImage)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - SetImage 실패: IconImage가 NULL"));
        return;
    }
    
    // 이미지 설정
    if (NewTexture)
    {
        // 브러시 생성 및 설정
        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.ImageSize = FVector2D(300.0f, 300.0f);
        Brush.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // 원래 색 그대로
        Brush.SetResourceObject(NewTexture);
        
        // 브러시를 이미지에 적용
        IconImage->SetBrush(Brush);
        
        // 확실하게 보이도록 설정
        IconImage->SetRenderOpacity(1.0f);
        IconImage->SetVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 이미지 설정 완료: %s"), *NewTexture->GetName());
        
        // 디버깅: 이미지 크기 로그 출력
        int32 Width = NewTexture->GetSizeX();
        int32 Height = NewTexture->GetSizeY();
        UE_LOG(LogTemp, Error, TEXT("텍스처 크기: %d x %d"), Width, Height);
    }
    else
    {
        // 빈 이미지 (빨간색 테두리로 표시)
        FSlateBrush EmptyBrush;
        EmptyBrush.DrawAs = ESlateBrushDrawType::Border;
        EmptyBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f); // 빨간색 반투명
        IconImage->SetBrush(EmptyBrush);
        
        UE_LOG(LogTemp, Error, TEXT("FruitUIWidget - 이미지 설정 실패: 텍스처 NULL"));
    }
    
    // 위치 업데이트
    UpdateWidgetPosition();
    
    // 명시적 가시성 설정
    this->SetVisibility(ESlateVisibility::Visible);
    IconImage->SetVisibility(ESlateVisibility::Visible);
}

void UFruitUIWidget::UpdateWidgetPosition()
{
    if (!IconImage) return;

    // IconImage가 담긴 CanvasPanelSlot 찾기
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconImage->Slot))
    {
        FVector2D DesiredPos(0.f, 0.f);

        switch (WidgetPosition)
        {
        case EWidgetPosition::LeftTop:
            DesiredPos = FVector2D(EdgePadding, EdgePadding);
            break;

        case EWidgetPosition::LeftMiddle:
            DesiredPos = FVector2D(EdgePadding, 400.f); // 적당한 값 예시
            break;

        case EWidgetPosition::RightTop:
            DesiredPos = FVector2D(800.f, EdgePadding); // 적당한 값 예시
            break;
        default:
            break;
        }

        CanvasSlot->SetPosition(DesiredPos);

        // 필요 시 크기나 Alignment도 조정
        CanvasSlot->SetAutoSize(true);
    }
}