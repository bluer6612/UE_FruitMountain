#include "TestUIWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"

UTestUIWidget::UTestUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    UE_LOG(LogTemp, Error, TEXT("테스트 위젯 생성자 호출!"));
}

void UTestUIWidget::NativePreConstruct()
{
    Super::NativePreConstruct();
    
    UE_LOG(LogTemp, Error, TEXT("테스트 위젯 NativePreConstruct 호출!"));
    
    // NativePreConstruct에서는 특별한 작업을 하지 않음
    // 이 함수는 주로 디자이너에서 설정된 속성을 런타임 전에 처리할 때 사용됨
}

void UTestUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UE_LOG(LogTemp, Error, TEXT(">>> TestUIWidget NativeConstruct 시작"));
    
    try
    {
        // 캔버스 패널 루트 확인
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
        if (!RootCanvas)
        {
            UE_LOG(LogTemp, Error, TEXT("루트 캔버스 없음 - 생성 시도"));
            
            // 새 캔버스 생성 및 설정
            RootCanvas = NewObject<UCanvasPanel>(this, TEXT("RootCanvas"));
            if (WidgetTree)
            {
                WidgetTree->RootWidget = RootCanvas;
                UE_LOG(LogTemp, Error, TEXT("새 루트 캔버스 설정 완료"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("위젯 트리가 NULL!"));
            }
        }
        
        // 간단한 빨간색 이미지 생성
        TestImage = NewObject<UImage>(this, TEXT("TestImage"));
        if (TestImage && RootCanvas)
        {
            UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(TestImage);
            
            // 큰 빨간색 상자
            FSlateBrush RedBrush;
            RedBrush.DrawAs = ESlateBrushDrawType::Box;
            RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
            TestImage->SetBrush(RedBrush);
            
            ImageSlot->SetPosition(FVector2D(200, 200));
            ImageSlot->SetSize(FVector2D(400, 400));
            ImageSlot->SetZOrder(9999);
            
            UE_LOG(LogTemp, Error, TEXT("빨간색 이미지 설정 완료"));
        }
        
        // 가시성 설정
        SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Error, TEXT("가시성 설정 완료"));
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("위젯 초기화 중 예외 발생: %s"), UTF8_TO_TCHAR(e.what()));
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("위젯 초기화 중 알 수 없는 예외 발생"));
    }
    
    UE_LOG(LogTemp, Error, TEXT(">>> TestUIWidget NativeConstruct 완료"));
}

void UTestUIWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 루트에서 제거하여 가비지 컬렉션 허용
    if (IsRooted())
    {
        RemoveFromRoot();
    }
    
    UE_LOG(LogTemp, Error, TEXT("테스트 위젯 NativeDestruct 호출!"));
}

void UTestUIWidget::ShowRedBlock()
{
    UE_LOG(LogTemp, Error, TEXT("ShowRedBlock 호출!"));
    
    // 이미 NativeConstruct에서 설정했지만 다시 한번 확인
    if (TestImage)
    {
        // 매우 큰 빨간색 상자로 설정 (화면 가득)
        FSlateBrush RedBrush;
        RedBrush.DrawAs = ESlateBrushDrawType::Box;
        RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
        TestImage->SetBrush(RedBrush);
        TestImage->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
        TestImage->SetRenderOpacity(1.0f);
        TestImage->SetVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogTemp, Error, TEXT("이미지 가시성 다시 설정!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TestImage가 NULL입니다!"));
    }
    
    // 위젯 자체의 가시성도 확인
    SetVisibility(ESlateVisibility::Visible);
}