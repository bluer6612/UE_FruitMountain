#include "MainMenuManager.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "MainMenuWidget.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "Interface/UI/TitleLevel/StartMenu/StartMenuWidget.h"

UMainMenuManager::UMainMenuManager()
{
    // 기본 초기화
}

void UMainMenuManager::Initialize(UMainMenuWidget* InOwner)
{
    Owner = InOwner;
    CurrentMenuIndex = 0;
    
    // 초기화 직후 메뉴 선택 업데이트
    UpdateMenuSelection();
}

void UMainMenuManager::BeginDestroy()
{
    Super::BeginDestroy();
}

bool UMainMenuManager::HandleKeyDown(const FKey& Key)
{
    bool bMoved = HandleMenuKey(Key, CurrentMenuIndex, MenuItemCount, [this]() { SelectCurrentMenu(); });
    if (bMoved)
    {
        UpdateMenuSelection();
    }
    return bMoved;
}

void UMainMenuManager::MoveSelectionUp()
{
    // 순환식으로 인덱스 감소
    CurrentMenuIndex = (CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();
}

void UMainMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();
}

void UMainMenuManager::SelectCurrentMenu()
{
    switch (CurrentMenuIndex)
    {
        case 0: // 게임 시작
            OpenStartMenu(); // OpenPlayLevel()에서 변경
            break;
            
        case 1: // 랭킹
            OpenRankingMenu();
            break;
            
        case 2: // 옵션
            OpenOptionsMenu();
            break;
            
        case 3: // 크레딧
            OpenCreditScreen();
            break;
    }
}

void UMainMenuManager::UpdateMenuSelection()
{
    if (!Owner || !Owner->MenuImage || !Owner->IndicatorAnimator)
        return;

    UImage* Indicator = Owner->IndicatorAnimator->GetIndicator();
    if (!Indicator)
        return;

    if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(Owner->MenuImage->Slot))
    {
        FVector2D MenuBasePos = MenuSlot->GetPosition();
        FVector2D TargetPos = {MenuBasePos.X + 50.f, MenuBasePos.Y - 255.f + 67.5f * CurrentMenuIndex};
        UE_LOG(LogTemp, Warning, TEXT("UpdateMenuSelection: CurrentMenuIndex=%d, MenuBasePos=(%.1f, %.1f), Indicator=%p"), CurrentMenuIndex, MenuBasePos.X, MenuBasePos.Y, Indicator);

        Owner->IndicatorAnimator->MoveToPosition(TargetPos);
    }
}

void UMainMenuManager::PlaySelectionAnimation()
{
    // 선택 효과는 UpdateMenuSelection에서 처리하므로 비워둠
    // 필요한 경우 소리 등의 추가 효과를 여기에 추가
}

// OpenPlayLevel은 미사용 상태로 남겨두거나 StartMenuWidget에서 호출할 수 있도록 함
void UMainMenuManager::OpenPlayLevel()
{
    // UI 애니메이션 중지
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    if (Owner->SelectIndicator)
    {
        Owner->SelectIndicator->SetRenderOpacity(0.0f);
    }
    
    // 위젯에 게임 시작 요청 위임
    if (Owner)
    {
        Owner->StartGame();
    }
    else
    {
        // 오류 상황 - 위젯 없이 직접 시도
        UE_LOG(LogTemp, Warning, TEXT("MainMenuManager: Owner가 없어 직접 레벨 전환 시도"));
        UGameplayStatics::OpenLevel(GetWorld(), TEXT("PlayLevel"));
    }
}

// 새로 추가된 함수: 게임 모드 선택 메뉴 열기
void UMainMenuManager::OpenStartMenu()
{
    // UI 애니메이션 중지
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    if (Owner->SelectIndicator)
    {
        Owner->SelectIndicator->SetRenderOpacity(0.0f);
    }
    
    // 게임 모드 선택 메뉴 생성 및 표시
    if (Owner)
    {
        UWorld* World = Owner->GetWorld();
        if (World)
        {
            UStartMenuWidget* StartMenu = CreateWidget<UStartMenuWidget>(World, UStartMenuWidget::StaticClass());
            if (StartMenu)
            {
                CurrentMenuIndex = 0;
                StartMenu->AddToViewport(9000);
                StartMenu->InitializeStartMenu();
                
                // 입력 모드 설정 (기존 타이틀 메뉴는 안 보이게)
                if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
                {
                    FInputModeUIOnly InputMode;
                    InputMode.SetWidgetToFocus(StartMenu->TakeWidget());
                    PC->SetInputMode(InputMode);
                }
                
                // 기존 타이틀 메뉴는 숨김
                if (Owner->MenuImage)
                {
                    Owner->MenuImage->SetVisibility(ESlateVisibility::Hidden);
                }
                if (Owner->LogoImage)
                {
                    Owner->LogoImage->SetVisibility(ESlateVisibility::Hidden);
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MainMenuManager: Owner가 없어 게임 모드 메뉴를 열 수 없음"));
    }
}

void UMainMenuManager::OpenRankingMenu()
{
    // 랭킹 화면 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("랭킹 메뉴 열기"));
}

void UMainMenuManager::OpenOptionsMenu()
{
    // 옵션 메뉴 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("옵션 메뉴 열기"));
}

void UMainMenuManager::OpenCreditScreen()
{
    // 크레딧 화면 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("크레딧 화면 열기"));
}

void UMainMenuManager::StartIndicatorAnimation(bool bStart)
{
    UTitleMenuManager::StartIndicatorAnimation(bStart);
}