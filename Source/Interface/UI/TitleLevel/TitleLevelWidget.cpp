#include "TitleLevelWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Kismet/GameplayStatics.h"

void UTitleLevelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 캔버스 패널 생성 또는 가져오기
    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!RootCanvas)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;
    }

    // 로고 이미지 위젯 생성 및 렌더러로 배치
    if (!LogoImage)
    {
        LogoImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LogoImage"));
        UUIWidgetRenderer::RenderUIImage(
            LogoImage,
            EWidgetAnchor::TopLeft,
            TEXT("/Game/UI/TitleLevel/UI_Title_Logo"),
            FVector2D(633.f, 369.f), // 실제 크기 사용
            80.f, 80.f
        );
        RootCanvas->AddChild(LogoImage);
    }

    // 메뉴 이미지 위젯 생성 및 렌더러로 배치
    if (!MenuImage)
    {
        MenuImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MenuImage"));
        UUIWidgetRenderer::RenderUIImage(
            MenuImage,
            EWidgetAnchor::BottomLeft,
            TEXT("/Game/UI/TitleLevel/UI_Title_Menu"),
            FVector2D(592.f, 359.f), // 실제 크기 사용
            80.f, 80.f
        );
        RootCanvas->AddChild(MenuImage);
    }

    UpdateMenuSelection();
}

FReply UTitleLevelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Up || Key == EKeys::W)
    {
        CurrentMenuIndex = FMath::Max(0, CurrentMenuIndex - 1);
        UpdateMenuSelection();
        return FReply::Handled();
    }
    if (Key == EKeys::Down || Key == EKeys::S)
    {
        CurrentMenuIndex = FMath::Min(1, CurrentMenuIndex + 1);
        UpdateMenuSelection();
        return FReply::Handled();
    }
    if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
    {
        OnMenuSelect();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTitleLevelWidget::UpdateMenuSelection()
{
    // 메뉴 선택 효과(예: 밝기, 크기 등) 구현 필요
}

void UTitleLevelWidget::OnMenuSelect()
{
    if (CurrentMenuIndex == 0)
    {
        UGameplayStatics::OpenLevel(this, TEXT("PlayLevel"));
    }
    else if (CurrentMenuIndex == 1)
    {
        UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
    }
}