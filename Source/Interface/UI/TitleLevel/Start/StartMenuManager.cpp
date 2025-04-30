#include "StartMenuManager.h"
#include "StartMenuWidget.h"
#include "Interface/UI/TitleLevel/Animator/MenuIndicatorAnimator.h"
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
    
    // 모드 이미지 업데이트
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
    if (!SelectIndicator || !Owner || !Owner->GameModeMenuImage || !IndicatorAnimator)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateMenuSelection: 필요한 객체가 nullptr"));
        return;
    }

    // 메뉴 항목별 위치 계산
    const float MenuItemBaseY = -30.f; // 첫 메뉴 항목의 Y 좌표 (중앙 기준)
    const float MenuItemSpacing = 70.f; // 메뉴 항목 간 간격
    
    // 선택된 메뉴 항목의 위치
    float targetY = MenuItemBaseY + (CurrentMenuIndex * MenuItemSpacing);
    
    // 인디케이터 위치 설정 (메뉴 왼쪽에 배치)
    FVector2D TargetPos = FVector2D(-230.f, targetY);
    
    // 애니메이터에게 위치 변경 요청
    IndicatorAnimator->MoveToPosition(TargetPos);
    
    // 모드 이미지도 함께 업데이트
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