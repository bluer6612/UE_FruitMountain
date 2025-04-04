// 새 파일: TestWidget.cpp
#include "TestWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h" // 이 헤더를 추가
#include "Blueprint/WidgetTree.h"

void UTestWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 캔버스 생성
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    WidgetTree->RootWidget = Canvas;
    
    // 텍스트 블록 생성
    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Text->SetText(FText::FromString(TEXT("테스트 UI 위젯입니다!")));
    Text->SetColorAndOpacity(FLinearColor::Red);
    Text->SetAutoWrapText(true);
    
    // 캔버스에 추가 (변수 이름 TextSlot으로 변경)
    UCanvasPanelSlot* TextSlot = Canvas->AddChildToCanvas(Text);
    TextSlot->SetPosition(FVector2D(200.0f, 200.0f));
    TextSlot->SetSize(FVector2D(400.0f, 400.0f));
}