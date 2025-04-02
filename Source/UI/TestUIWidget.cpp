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
    
    UE_LOG(LogTemp, Error, TEXT("테스트 위젯 NativeConstruct 호출!"));
    
    // 단순한 캔버스 패널 생성
    UCanvasPanel* RootCanvas = NewObject<UCanvasPanel>(this, TEXT("RootCanvas"));
    TestImage = NewObject<UImage>(this, TEXT("TestImage"));
    
    // 캔버스를 루트로 설정 - RootWidget 직접 접근 대신 WidgetTree 사용
    if (!GetRootWidget() && WidgetTree)
    {
        WidgetTree->RootWidget = RootCanvas;
    }
    
    // 이미지를 캔버스에 추가
    if (RootCanvas && TestImage)
    {
        UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(TestImage);
        
        // 큰 빨간색 상자 설정
        FSlateBrush RedBrush;
        RedBrush.DrawAs = ESlateBrushDrawType::Box;
        RedBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
        TestImage->SetBrush(RedBrush);
        TestImage->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
        
        // 전체 화면 크기로 설정
        ImageSlot->SetPosition(FVector2D(400, 300));
        ImageSlot->SetSize(FVector2D(800, 600));
        ImageSlot->SetZOrder(9999);
        
        UE_LOG(LogTemp, Error, TEXT("큰 빨간색 이미지 생성 완료!"));
    }
    
    // 명시적 가시성 설정
    SetVisibility(ESlateVisibility::Visible);
    
    UE_LOG(LogTemp, Error, TEXT("테스트 위젯 초기화 완료!"));
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