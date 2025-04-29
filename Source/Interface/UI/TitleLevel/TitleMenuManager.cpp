#include "TitleMenuManager.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TitleLevelWidget.h"
#include "MenuIndicatorAnimator.h"

UTitleMenuManager::UTitleMenuManager()
{
    // 기본 초기화
}

void UTitleMenuManager::Initialize(UImage* InSelectIndicator, UTitleLevelWidget* InOwner)
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
}

void UTitleMenuManager::BeginDestroy()
{
    // 정리 작업
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(false);
    }
    
    Super::BeginDestroy();
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
    PlaySelectionAnimation();
}

void UTitleMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();
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
    if (!SelectIndicator || !Owner || !Owner->MenuImage || !IndicatorAnimator)
    {
        return;
    }

    // 위치 계산
    if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(Owner->MenuImage->Slot))
    {
        FVector2D MenuBasePos = MenuSlot->GetPosition();
        FVector2D TargetPos = {MenuBasePos.X + 50.f, MenuBasePos.Y - 255.f + 67.5f * CurrentMenuIndex};

        // 애니메이터에게 위치 변경 요청
        IndicatorAnimator->MoveToPosition(TargetPos);
    }
}

void UTitleMenuManager::PlaySelectionAnimation()
{
    // 선택 효과는 UpdateMenuSelection에서 처리하므로 비워둠
    // 필요한 경우 소리 등의 추가 효과를 여기에 추가
}

void UTitleMenuManager::OpenPlayLevel()
{
    // 안전한 레벨 전환
    if (Owner && Owner->GetWorld())
    {
        APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
        
        if (PC)
        {
            // 입력 비활성화로 추가 입력 방지
            PC->DisableInput(PC);
            
            // 페이드 아웃 효과 추가 (선택 사항)
            if (IndicatorAnimator)
            {
                IndicatorAnimator->StartAnimation(false);
            }
            
            if (SelectIndicator)
            {
                SelectIndicator->SetRenderOpacity(0.0f);
            }
            
            // 약간의 지연 후 레벨 열기
            FTimerHandle DelayHandle;
            Owner->GetWorld()->GetTimerManager().SetTimer(
                DelayHandle, 
                [this]()
                {
                    if (Owner && Owner->GetWorld())
                    {
                        UGameplayStatics::OpenLevel(Owner->GetWorld(), TEXT("PlayLevel"));
                    }
                }, 
                0.1f, 
                false
            );
        }
        else
        {
            UGameplayStatics::OpenLevel(Owner->GetWorld(), TEXT("PlayLevel"));
        }
    }
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

void UTitleMenuManager::StartIndicatorAnimation(bool bStart)
{
    // 애니메이션 관리자를 통해 애니메이션 제어
    if (IndicatorAnimator)
    {
        IndicatorAnimator->StartAnimation(bStart);
    }
}