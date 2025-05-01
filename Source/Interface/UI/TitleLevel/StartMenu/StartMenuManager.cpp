#include "StartMenuManager.h"
#include "StartMenuWidget.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"

UStartMenuManager::UStartMenuManager()
{
}

void UStartMenuManager::Initialize(UStartMenuWidget* InOwner)
{
    Owner = InOwner;
    Owner->CurrentMenuIndex = 0;
    UpdateMenuSelection();
}

void UStartMenuManager::BeginDestroy()
{
    Super::BeginDestroy();
}

bool UStartMenuManager::HandleKeyDown(const FKey& Key)
{
    if (!Owner)
    {
        return false;
    }

    bool bMoved = Owner->HandleMenuKey(Key, Owner->CurrentMenuIndex, MenuItemCount, [this]() { SelectCurrentMenu(); });
    if (bMoved)
    {
        UpdateMenuSelection();
    }
    return bMoved;
}

void UStartMenuManager::MoveSelectionUp()
{
    Owner->CurrentMenuIndex = (Owner->CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();
}

void UStartMenuManager::MoveSelectionDown()
{
    Owner->CurrentMenuIndex = (Owner->CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();
}

void UStartMenuManager::SelectCurrentMenu()
{
    switch (Owner->CurrentMenuIndex)
    {
        case 0: // 기본 모드
            SelectClassicMode();
            break;
        case 1: // 시간 제한 모드
            SelectTimeLimitMode();
            break;
        case 2: // 뒤로 가기
            BackToMainMenu();
            break;
    }
}

void UStartMenuManager::UpdateMenuSelection()
{

    if (!Owner || !Owner->StartMenuImage || !Owner->IndicatorAnimator)
    {
        return;
    }

    UImage* Indicator = Owner->IndicatorAnimator->GetIndicator();
    if (!Indicator)
    {
        return;
    }

    if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(Owner->StartMenuImage->Slot))
    {
        FVector2D MenuBasePos = MenuSlot->GetPosition();
        FVector2D TargetPos = {MenuBasePos.X - 600.f, MenuBasePos.Y - 67.5f + 67.5f * Owner->CurrentMenuIndex};
        Owner->IndicatorAnimator->MoveToPosition(TargetPos);
    }
}

void UStartMenuManager::PlaySelectionAnimation()
{
    // 선택 효과는 UpdateMenuSelection에서 처리하므로 비워둠
    // 필요한 경우 소리 등의 추가 효과를 여기에 추가
}

void UStartMenuManager::SelectClassicMode()
{
    if (Owner && Owner->IndicatorAnimator)
    {
        Owner->IndicatorAnimator->EndAnimation();
        Owner->UTitleLevelWidget::PlayFadeOut();
        Owner->StartGame();
    }
}

void UStartMenuManager::SelectTimeLimitMode()
{
    if (Owner && Owner->IndicatorAnimator)
    {
        Owner->IndicatorAnimator->EndAnimation();
    }
}

void UStartMenuManager::BackToMainMenu()
{
    if (Owner && Owner->IndicatorAnimator)
    {
        Owner->IndicatorAnimator->EndAnimation();
        Owner->CurrentMenuIndex = 0;
        Owner->BackToMainMenu();
    }
}

void UStartMenuManager::UpdateGameModeImage()
{
    if (!Owner || !Owner->StartMenuImage)
    {
        return;
    }

    FString TexturePath;
    switch (Owner->CurrentMenuIndex)
    {
        case 0:
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1");
            break;
        case 1:
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode2");
            break;
        case 2:
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode3");
            break;
        default:
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1");
            break;
    }

    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (LoadedTexture)
    {
        FSlateBrush Brush = Owner->StartMenuImage->GetBrush();
        Brush.SetResourceObject(LoadedTexture);
        Owner->StartMenuImage->SetBrush(Brush);
    }
}