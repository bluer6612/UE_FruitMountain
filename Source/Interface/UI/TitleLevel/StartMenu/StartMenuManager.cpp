#include "StartMenuManager.h"
#include "StartMenuWidget.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"

UStartMenuManager::UStartMenuManager()
{
    // 기본 초기화
}

void UStartMenuManager::Initialize(UStartMenuWidget* InOwner)
{
    Owner = InOwner;
    CurrentMenuIndex = 0;
    UpdateMenuSelection();
    UpdateGameModeImage();
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

void UStartMenuManager::PlaySelectionAnimation()
{
    // 선택 효과 재생
    // 필요한 경우 사운드 효과 추가 가능
    if (Owner && IndicatorAnimator)
    {
        // 짧은 선택 애니메이션 효과
    }
}

bool UStartMenuManager::HandleKeyDown(const FKey& Key)
{
    return HandleMenuKey(Key, CurrentMenuIndex, MenuItemCount, [this]() { SelectCurrentMenu(); });
}

void UStartMenuManager::MoveSelectionUp()
{
    // 순환식으로 인덱스 감소
    CurrentMenuIndex = (CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
    UpdateMenuSelection(); // 이것만 호출하면 내부에서 UpdateGameModeImage() 호출함
    PlaySelectionAnimation();
}

void UStartMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection(); // 이것만 호출하면 내부에서 UpdateGameModeImage() 호출함
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
    if (!Owner || !Owner->GameModeMenuImage || !Owner->IndicatorAnimator)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateMenuSelection: 필요한 객체가 nullptr"));
        return;
    }

    UImage* Indicator = Owner->IndicatorAnimator->GetIndicator();
    if (!Indicator)
    {
        return;
    }

    // 위치 계산 및 이동
    const float MenuItemBaseY = -30.f;
    const float MenuItemSpacing = 70.f;
    float targetY = MenuItemBaseY + (CurrentMenuIndex * MenuItemSpacing);
    FVector2D TargetPos = FVector2D(-230.f, targetY);

    Owner->IndicatorAnimator->MoveToPosition(TargetPos);
    UpdateGameModeImage();
}

// GameModeDescImage 대신 GameModeMenuImage의 텍스처를 교체하는 함수로 변경
void UStartMenuManager::UpdateGameModeImage()
{
    if (!Owner || !Owner->GameModeMenuImage)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateGameModeImage: Owner 또는 GameModeMenuImage가 nullptr"));
        return;
    }
    
    // 현재 메뉴 선택에 따라 이미지 변경
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
        FSlateBrush Brush = Owner->GameModeMenuImage->GetBrush();
        Brush.SetResourceObject(LoadedTexture);
        Owner->GameModeMenuImage->SetBrush(Brush);
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
        CurrentMenuIndex = 0; // 메인 메뉴 복귀 시 인덱스 초기화
        Owner->BackToMainMenu();
    }
}

void UStartMenuManager::StartIndicatorAnimation(bool bStart)
{
    UTitleMenuManager::StartIndicatorAnimation(bStart);
}