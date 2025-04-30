#include "StartMenuManager.h"
#include "StartMenuWidget.h"
#include "Interface/UI/TitleLevel/Default/MenuIndicatorAnimator.h"
#include "Interface/UI/TitleLevel/Default/TitleLevelWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"

UStartMenuManager::UStartMenuManager()
{
    // 기본 초기화
}

void UStartMenuManager::Initialize(UImage* InSelectIndicator, UStartMenuWidget* InOwner)
{
    SelectIndicator = InSelectIndicator;
    Owner = InOwner;
    CurrentMenuIndex = 0;
    
    // 애니메이션 관리자 생성 및 초기화
    IndicatorAnimator = NewObject<UMenuIndicatorAnimator>(this);
    if (IndicatorAnimator)
    {
        IndicatorAnimator->Initialize(SelectIndicator);
    }
    
    // 초기화 직후 메뉴 선택 업데이트
    UpdateMenuSelection();
    
    // 설명 이미지 초기화
    UpdateGameModeDescription();
}

void UStartMenuManager::BeginDestroy()
{
    // 정리 작업
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    Super::BeginDestroy();
}

bool UStartMenuManager::HandleKeyDown(const FKey& Key)
{
    // 위로 이동 (UP, W)
    if (Key == EKeys::Up || Key == EKeys::W)
    {
        MoveSelectionUp();
        return true;
    }
    
    // 아래로 이동 (DOWN, S)
    if (Key == EKeys::Down || Key == EKeys::S)
    {
        MoveSelectionDown();
        return true;
    }
    
    // 선택 (Enter, SpaceBar)
    if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
    {
        SelectCurrentMenu();
        return true;
    }
    
    // 뒤로 가기 (Escape)
    if (Key == EKeys::Escape)
    {
        BackToMainMenu();
        return true;
    }
    
    // 처리되지 않은 키
    return false;
}

void UStartMenuManager::MoveSelectionUp()
{
    // 순환식으로 인덱스 감소
    CurrentMenuIndex = (CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
    UpdateMenuSelection();
    UpdateGameModeDescription();
    PlaySelectionAnimation();
}

void UStartMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
    UpdateGameModeDescription();
    PlaySelectionAnimation();
}

void UStartMenuManager::SelectCurrentMenu()
{
    switch (CurrentMenuIndex)
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
    if (!SelectIndicator || !Owner || !IndicatorAnimator)
    {
        return;
    }

    // 위치 계산 (게임 모드 메뉴에 맞게 조정)
    if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(Owner->GameModeMenuImage->Slot))
    {
        FVector2D MenuBasePos = MenuSlot->GetPosition();
        
        // 각 메뉴 항목의 위치 (조정 필요)
        float BaseY = MenuBasePos.Y + 70.f;
        float ItemSpacing = 75.f;
        FVector2D TargetPos = {MenuBasePos.X + 50.f, BaseY + ItemSpacing * CurrentMenuIndex};

        // 애니메이터에게 위치 변경 요청
        IndicatorAnimator->MoveToPosition(TargetPos);
    }
}

void UStartMenuManager::PlaySelectionAnimation()
{
    // 선택 효과는 UpdateMenuSelection에서 처리하므로 비워둠
    // 필요한 경우 소리 등의 추가 효과를 여기에 추가
}

void UStartMenuManager::UpdateGameModeDescription()
{
    if (!Owner || !Owner->GameModeDescImage)
    {
        return;
    }
    
    // 현재 메뉴 선택에 따라 설명 이미지 변경
    FString TexturePath;
    switch (CurrentMenuIndex)
    {
        case 0: // 기본 모드
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1");
            break;
            
        case 1: // 시간 제한 모드
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode2");
            break;
            
        case 2: // 뒤로 가기
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode3");
            break;
            
        default:
            TexturePath = TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1");
            break;
    }
    
    // 텍스처 로드 및 설정
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (LoadedTexture)
    {
        FSlateBrush Brush = Owner->GameModeDescImage->GetBrush();
        Brush.SetResourceObject(LoadedTexture);
        Owner->GameModeDescImage->SetBrush(Brush);
    }
}

void UStartMenuManager::SelectClassicMode()
{
    // UI 애니메이션 중지
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    // 기본 모드로 게임 시작
    if (Owner)
    {
        Owner->StartGame(0);
    }
}

void UStartMenuManager::SelectTimeLimitMode()
{
    // UI 애니메이션 중지
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    // 시간 제한 모드로 게임 시작
    if (Owner)
    {
        Owner->StartGame(1);
    }
}

void UStartMenuManager::BackToMainMenu()
{
    // UI 애니메이션 중지
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    // 메인 메뉴로 돌아가기
    if (Owner)
    {
        Owner->BackToMainMenu();
    }
}

void UStartMenuManager::StartIndicatorAnimation(bool bStart)
{
    // 애니메이션 관리자를 통해 애니메이션 제어
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(bStart);
    }
}