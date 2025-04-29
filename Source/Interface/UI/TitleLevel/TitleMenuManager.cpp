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
            // 첫 애니메이션은 0.5초 후 시작
            if (UWorld* World = Owner ? Owner->GetWorld() : nullptr)
            {
                FTimerHandle FirstAnimHandle;
                World->GetTimerManager().SetTimer(FirstAnimHandle, [this]()
                {
                    // 첫 애니메이션 실행
                    PlayIndicatorAnimation();
                    
                    // 이후 반복 실행 설정
                    StartIndicatorAnimation();
                }, 0.25f, false);
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

// 이징 함수 개선 - 더 자연스러운 바운스 효과
namespace AnimationHelper
{
    // 사인파 진동을 이용한 이징 함수 - 돌아오는 과정에서 여러번 진동
    static float ElasticEaseOut(float t)
    {
        const float c4 = (2.0f * PI) / 3.0f;
        
        if (t == 0.0f)
            return 0.0f;
        if (t == 1.0f)
            return 1.0f;
            
        // 진동하는 돌아오는 움직임
        return FMath::Pow(2.0f, -10.0f * t) * FMath::Sin((t * 10.0f - 0.75f) * c4) + 1.0f;
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
    
    // 애니메이션 지속 시간 3초로 설정
    const float AnimDuration = 3.0f;
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
            
            if (Progress <= 0.4f)
            {
                // 첫 40%: 왼쪽으로 이동하면서 밝아짐
                float NormalizedProgress = Progress / 0.4f; // 0.0 ~ 1.0
                
                // 부드러운 가속
                float EasedProgress = FMath::Pow(NormalizedProgress, 2.0f);
                
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
                // 나머지 60%: 돌아오는 과정에서 바운싱
                float NormalizedProgress = (Progress - 0.4f) / 0.6f; // 0.0 ~ 1.0
                
                // 탄성 효과 적용 - 돌아오면서 여러번 진동
                float EasedProgress = AnimationHelper::ElasticEaseOut(NormalizedProgress);
                
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    // 진동하며 원래 위치로 복귀 (-15에서 0까지)
                    float NewX = CurrentPos.X - 15.f * (1.0f - EasedProgress);
                    IndicatorSlot->SetPosition(FVector2D(NewX, CurrentPos.Y));
                }
                
                // 색상 변화도 탄성 효과와 함께 진동
                float BrightnessFactor = 1.0f + 0.5f * (1.0f - EasedProgress);
                // 탄성 효과에 의해 약간의 오버슈트 허용 (0.9~1.1 범위로 제한)
                BrightnessFactor = FMath::Clamp(BrightnessFactor, 0.9f, 1.1f);
                WeakIndicator->SetColorAndOpacity(FLinearColor(BrightnessFactor, BrightnessFactor, BrightnessFactor));
            }
            
            // 애니메이션 완료
            if (Progress >= 1.0f)
            {
                // 색상만 원래대로 복원
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