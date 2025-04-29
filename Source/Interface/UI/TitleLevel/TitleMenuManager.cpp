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
        
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            // 첫 애니메이션은 0.5초 후 시작하고 그 이후부터는 1.5초마다 재생
            if (UWorld* World = Owner ? Owner->GetWorld() : nullptr)
            {
                // 0.5초 후 첫 애니메이션 실행
                FTimerHandle FirstAnimHandle;
                World->GetTimerManager().SetTimer(FirstAnimHandle, [this]()
                {
                    // 첫 애니메이션 실행
                    PlayIndicatorAnimation();
                    
                    // 이후 1.5초마다 반복 실행 설정
                    StartIndicatorAnimation();
                }, 0.5f, false);
            }
        }
    }
}

void UTitleMenuManager::StartIndicatorAnimation()
{
    if (UWorld* World = Owner ? Owner->GetWorld() : nullptr)
    {
        // 1.5초마다 애니메이션 시작 (두 번째 애니메이션부터)
        World->GetTimerManager().SetTimer(IndicatorAnimationTimerHandle, this, 
            &UTitleMenuManager::PlayIndicatorAnimation, IndicatorAnimationInterval, true);
    }
}

// 이징 함수 추가 (클래스 외부 또는 헤더에 정적 함수로 추가)
namespace AnimationHelper
{
    // 바운스 효과가 있는 이징 함수
    static float BounceEaseOut(float t)
    {
        // 바운스 효과 (목표에 닿으면 살짝 튕겨나감)
        if (t < 0.5f)
        {
            // 처음에는 천천히 가속
            return 4 * t * t * t;
        }
        else
        {
            // 후반부에는 오버슈트 후 진동하며 정착
            float f = ((2 * t) - 2);
            return 0.5f * f * f * f * f * f + 1;
        }
    }
    
    // 오버슈트 효과 (목표를 약간 지나친 후 돌아옴)
    static float BackEaseOut(float t)
    {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1;
        
        return 1 + c3 * FMath::Pow(t - 1, 3) + c1 * FMath::Pow(t - 1, 2);
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
    
    // 현재 위치 저장
    FVector2D CurrentPos = IndicatorSlot->GetPosition();
    
    FTimerHandle* AnimHandle = new FTimerHandle();
    
    if (UWorld* World = SelectIndicator->GetWorld())
    {
        World->GetTimerManager().SetTimer(*AnimHandle, [WeakThis, WeakIndicator, AnimDuration, TickInterval, ElapsedTime, AnimHandle, CurrentPos]()
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
            float Progress = FMath::Clamp(*ElapsedTime / AnimDuration, 0.0f, 1.0f);
            
            if (Progress <= 0.5f)
            {
                // 첫 절반: 왼쪽으로 이동하면서 밝아짐
                float NormalizedProgress = Progress * 2.0f; // 0.0 ~ 1.0
                
                // 바운스 효과 적용
                float EasedProgress = AnimationHelper::BounceEaseOut(NormalizedProgress);
                
                // 위치 이동 (왼쪽으로 15.f까지)
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    float NewX = CurrentPos.X - 15.f * EasedProgress;
                    IndicatorSlot->SetPosition(FVector2D(NewX, CurrentPos.Y));
                }
                
                // 밝기 증가
                float Brightness = 1.0f + 0.5f * EasedProgress;
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            else
            {
                // 후반 절반: 다시 오른쪽으로 이동하여 원래 위치로 복귀
                float NormalizedProgress = (Progress - 0.5f) * 2.0f; // 0.0 ~ 1.0
                
                // 바운스 효과 적용 (돌아올 때는 좀 더 빠르게 시작했다가 감속)
                float EasedProgress = AnimationHelper::BackEaseOut(NormalizedProgress);
                
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    // 왼쪽에서 시작해 오른쪽으로 이동하며 중간에 살짝 오버슈트
                    float NewX = (CurrentPos.X - 15.f) + 15.f * EasedProgress;
                    
                    // 오버슈트 효과 (목표보다 살짝 더 움직였다가 돌아옴)
                    if (NormalizedProgress > 0.8f && NormalizedProgress < 0.95f)
                    {
                        NewX += 3.0f * (NormalizedProgress - 0.8f) * 6.67f;
                    }
                    else if (NormalizedProgress >= 0.95f)
                    {
                        NewX += 3.0f * (1.0f - NormalizedProgress) * 60.0f;
                    }
                    
                    IndicatorSlot->SetPosition(FVector2D(NewX, CurrentPos.Y));
                }
                
                // 색상 원래대로 (부드럽게 감소)
                float Brightness = 1.5f - 0.5f * EasedProgress;
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            
            // 애니메이션 완료
            if (Progress >= 1.0f)
            {
                // 원래 위치로 복원 확인
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    IndicatorSlot->SetPosition(CurrentPos);
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
            FVector2D MenuBasePos = MenuSlot->GetPosition();
            FVector2D TargetPos = {MenuBasePos.X + 37.5f, MenuBasePos.Y - 250.f + 50.f * CurrentMenuIndex};

            if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
            {
                IndicatorSlot->SetPosition(TargetPos);
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