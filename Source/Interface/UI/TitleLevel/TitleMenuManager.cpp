#include "TitleMenuManager.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TitleLevelWidget.h"

UTitleMenuManager::UTitleMenuManager()
{
    // 기본 초기화
}

void UTitleMenuManager::Initialize(UImage* InSelectIndicator, UTitleLevelWidget* InOwner)
{
    SelectIndicator = InSelectIndicator;
    Owner = InOwner;
    CurrentMenuIndex = 0;
    
    // 초기화 직후 메뉴 선택 업데이트
    UpdateMenuSelection();
}

bool UTitleMenuManager::HandleKeyDown(const FKey& Key)
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
    
    // 처리되지 않은 키
    return false;
}

void UTitleMenuManager::MoveSelectionUp()
{
    // 순환식으로 인덱스 감소
    CurrentMenuIndex = (CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
    UpdateMenuSelection();
}

void UTitleMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
}

void UTitleMenuManager::SelectCurrentMenu()
{
    switch (CurrentMenuIndex)
    {
        case 0: // 게임 시작
            OpenPlayLevel();
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

void UTitleMenuManager::UpdateMenuSelection()
{
    if (!SelectIndicator)
    {
        return;
    }
    
    // 현재 선택된 메뉴 항목에 표시기 위치 설정
    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
    {
        // X 위치는 메뉴 위치(150.f)에서 75.f 뺀 값으로 고정
        float IndicatorX = 150.f - 75.f;
        float TargetY = MenuPositions[CurrentMenuIndex];
        
        // 위치 설정
        FVector2D NewPos(IndicatorX, TargetY);
        IndicatorSlot->SetPosition(NewPos);
        
        // 애니메이션 효과
        PlaySelectionAnimation();
    }
}

void UTitleMenuManager::PlaySelectionAnimation()
{
    if (!SelectIndicator) return;
    
    // 크기 확대 애니메이션
    SelectIndicator->SetRenderScale(FVector2D(1.2f, 1.2f));
    
    // 타이머를 사용하여 원래 크기로 복원
    FTimerHandle ResetHandle;
    if (UWorld* World = SelectIndicator->GetWorld())
    {
        World->GetTimerManager().SetTimer(ResetHandle, [this]()
        {
            if (SelectIndicator)
            {
                SelectIndicator->SetRenderScale(FVector2D(1.0f, 1.0f));
            }
        }, 0.15f, false);
    }
}

void UTitleMenuManager::OpenPlayLevel()
{
    UGameplayStatics::OpenLevel(Owner, TEXT("PlayLevel"));
}

void UTitleMenuManager::OpenRankingMenu()
{
    // 랭킹 화면 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("랭킹 메뉴 열기"));
}

void UTitleMenuManager::OpenOptionsMenu()
{
    // 옵션 메뉴 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("옵션 메뉴 열기"));
}

void UTitleMenuManager::OpenCreditScreen()
{
    // 크레딧 화면 표시 로직
    UE_LOG(LogTemp, Warning, TEXT("크레딧 화면 열기"));
}