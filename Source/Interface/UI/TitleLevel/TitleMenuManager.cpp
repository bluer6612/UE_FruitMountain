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
    
    // 초기화 시 SelectIndicator가 명확히 보이도록 함
    if (SelectIndicator)
    {
        SelectIndicator->SetRenderOpacity(1.0f);
        
        // SelectIndicator의 원래 위치 저장 (애니메이션을 위해)
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            // 원래 위치 저장
            IndicatorOriginalPosition = IndicatorSlot->GetPosition();
            
            // 애니메이션 타이머 설정 (1.5초 후 첫 애니메이션 시작)
            StartIndicatorAnimation();
        }
    }
}

void UTitleMenuManager::StartIndicatorAnimation()
{
    if (UWorld* World = Owner ? Owner->GetWorld() : nullptr)
    {
        // 1.5초마다 애니메이션 시작
        World->GetTimerManager().SetTimer(IndicatorAnimationTimerHandle, this, 
            &UTitleMenuManager::PlayIndicatorAnimation, IndicatorAnimationInterval, true);
    }
}

void UTitleMenuManager::PlayIndicatorAnimation()
{
    if (!SelectIndicator)
    {
        return;
    }
    
    UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot);
    if (!IndicatorSlot)
    {
        return;
    }
    
    // 애니메이션을 위한 임시 변수
    const float AnimDuration = IndicatorAnimationDuration;
    const float TickInterval = 0.016f; // 약 60fps
    float* ElapsedTime = new float(0.0f);
    
    TWeakObjectPtr<UTitleMenuManager> WeakThis(this);
    TWeakObjectPtr<UImage> WeakIndicator(SelectIndicator);
    FVector2D OrigPos = IndicatorOriginalPosition;
    
    FTimerHandle* AnimHandle = new FTimerHandle();
    
    if (UWorld* World = SelectIndicator->GetWorld())
    {
        World->GetTimerManager().SetTimer(*AnimHandle, [WeakThis, WeakIndicator, AnimDuration, TickInterval, ElapsedTime, AnimHandle, OrigPos]()
        {
            if (!WeakThis.IsValid() || !WeakIndicator.IsValid())
            {
                if (UWorld* TimerWorld = GEngine && WeakThis.IsValid() ? 
                    GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()) : nullptr)
                {
                    TimerWorld->GetTimerManager().ClearTimer(*AnimHandle);
                }
                delete AnimHandle;
                delete ElapsedTime;
                return;
            }
            
            *ElapsedTime += TickInterval;
            float Progress = *ElapsedTime / AnimDuration;
            
            if (Progress <= 0.5f)
            {
                // 첫 절반: 왼쪽으로 이동하면서 밝아짐
                float NormalizedProgress = Progress * 2.0f; // 0.0 ~ 1.0
                
                // 위치 이동 (최대 20.f 왼쪽으로)
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    float NewX = OrigPos.X - 20.0f * NormalizedProgress;
                    IndicatorSlot->SetPosition(FVector2D(NewX, OrigPos.Y));
                }
                
                // 색상 밝아짐 (1.0 -> 1.5)
                float Brightness = 1.0f + 0.5f * NormalizedProgress;
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            else
            {
                // 후반 절반: 오른쪽으로 돌아오면서 어두워짐
                float NormalizedProgress = (Progress - 0.5f) * 2.0f; // 0.0 ~ 1.0
                
                // 위치 이동 (원래 위치로 복귀)
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    float NewX = OrigPos.X - 10.0f * (1.0f - NormalizedProgress);
                    IndicatorSlot->SetPosition(FVector2D(NewX, OrigPos.Y));
                }
                
                // 색상 원래대로 (1.5 -> 1.0)
                float Brightness = 1.0f + 0.5f * (1.0f - NormalizedProgress);
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            
            // 애니메이션 완료
            if (Progress >= 1.0f)
            {
                // 원래 상태로 복원
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    IndicatorSlot->SetPosition(OrigPos);
                }
                WeakIndicator->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
                
                if (UWorld* TimerWorld = GEngine && WeakThis.IsValid() ? 
                    GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()) : nullptr)
                {
                    TimerWorld->GetTimerManager().ClearTimer(*AnimHandle);
                }
                delete AnimHandle;
                delete ElapsedTime;
            }
        }, TickInterval, true);
    }
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
    PlaySelectionAnimation();  // 애니메이션 효과 추가
}

void UTitleMenuManager::MoveSelectionDown()
{
    // 순환식으로 인덱스 증가
    CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
    UpdateMenuSelection();
    PlaySelectionAnimation();  // 애니메이션 효과 추가
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

    if (Owner && Owner->MenuImage)
    {
        if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(Owner->MenuImage->Slot))
        {
            float MenuBaseY = MenuSlot->GetPosition().Y; // 메뉴 이미지의 Y 위치
            float IndicatorX = MenuBaseY - 350.f;
            float TargetY = MenuBaseY - 250.f + 50.f * CurrentMenuIndex; // 메뉴 기준 + 50f씩 증가

            if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
            {
                IndicatorSlot->SetPosition(FVector2D(IndicatorX, TargetY));
            }
        }
    }
}

void UTitleMenuManager::PlaySelectionAnimation()
{
    if (!SelectIndicator)
    {
        return;
    }
    
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