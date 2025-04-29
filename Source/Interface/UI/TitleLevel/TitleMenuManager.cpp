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
                    // 첫 애니메이션만 실행 (이후에는 자동 반복됨)
                    PlayIndicatorAnimation();
                }, 0.25f, false);
            }
        }
    }
}

void UTitleMenuManager::StartIndicatorAnimation(bool bStart)
{
    IsIndicatorAnimating = bStart;
    
    if (bStart && !IsAnimationRunning)
    {
        PlayIndicatorAnimation();
    }
}

// 이징 함수 개선 - 더 자연스러운 바운스 효과
namespace AnimationHelper
{
    // 빠른 바운싱 효과 (왼쪽으로 이동할 때)
    static float FastBounceEasing(float t)
    {
        // 빠른 진동 주파수(20Hz)를 가진 사인파 + 선형 움직임
        const float frequency = 20.0f;
        float oscillation = FMath::Sin(t * frequency) * (1.0f - t) * 0.2f; // 진폭은 시간에 따라 감소
        return t + oscillation; // 기본 진행에 진동 추가
    }
    
    // 느린 탄성 효과 (오른쪽으로 돌아올 때)
    static float SlowElasticEaseOut(float t)
    {
        const float c4 = (2.0f * PI) / 4.5f; // 진동 주파수 감소
        
        if (t == 0.0f)
            return 0.0f;
        if (t == 1.0f)
            return 1.0f;
            
        // 진동하는 돌아오는 움직임 (진폭을 크게, 주파수는 낮게)
        return FMath::Pow(2.0f, -7.0f * t) * FMath::Sin((t - 0.1f) * c4) + 1.0f;
    }
}

void UTitleMenuManager::PlayIndicatorAnimation()
{
    IsAnimationRunning = true; // 애니메이션 시작 플래그 설정
    
    if (!SelectIndicator)
    {
        return;
    }
    
    UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot);
    if (!IndicatorSlot)
    {
        return;
    }
    
    // 애니메이션 지속 시간 설정
    const float AnimDuration = IndicatorAnimationDuration;
    const float TickInterval = 0.006f; // 약 166fps로 증가 (더 부드러운 애니메이션)
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
            
            // 왼쪽으로 이동하는 구간 (13.33%)
            if (Progress <= 0.1333f)
            {
                // 첫 13.33%: 원래 위치에서 왼쪽으로 더 빠르게 튕겨나감
                float NormalizedProgress = Progress / 0.1333f; // 0.0 ~ 1.0
                
                // 가속되는 움직임 (easeOut)
                float EasedProgress = 1.0f - FMath::Pow(1.0f - NormalizedProgress, 2.0f);
                
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    // 왼쪽으로 15.f 이동 (Y축 변화 없음)
                    float NewX = CurrentPos.X - 15.f * EasedProgress;
                    IndicatorSlot->SetPosition(FVector2D(NewX, CurrentPos.Y));
                }
                
                // 밝기 증가
                float Brightness = 1.0f + 0.5f * EasedProgress;
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            // 오른쪽으로 돌아오는 구간 (23.33%)
            else if (Progress <= 0.3666f)
            {
                // 오른쪽으로 돌아오는 시간
                float NormalizedProgress = (Progress - 0.1333f) / 0.2333f; // 0.0 ~ 1.0
                
                // 천천히 시작하여 점점 가속 (easeIn)
                float EasedProgress = NormalizedProgress * NormalizedProgress;
                
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    // 왼쪽에서 원래 위치로 빠르게 복귀 (Y축 변화 없음)
                    float NewX = (CurrentPos.X - 15.f) + 15.f * EasedProgress;
                    IndicatorSlot->SetPosition(FVector2D(NewX, CurrentPos.Y));
                }
                
                // 밝기 감소
                float Brightness = 1.5f - 0.5f * EasedProgress;
                WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
            }
            // 대기 시간 대폭 줄임 - 전체의 10%만 대기 (기존 63.34% → 10%)
            else if (Progress <= 0.4666f) // 원위치에 0.1(10%)만 대기
            {
                // 원위치 유지
                if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                {
                    IndicatorSlot->SetPosition(CurrentPos);
                }
                
                // 색상은 원래대로
                WeakIndicator->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
            }
            // 애니메이션 조기 완료 처리
            else
            {
                // 애니메이션을 조기에 완료로 간주하고 타이머 정리
                if (UWorld* TimerWorld = GEngine && WeakThis.IsValid() ? 
                    GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()) : nullptr)
                {
                    TimerWorld->GetTimerManager().ClearTimer(*AnimHandle);
                }
                
                delete AnimHandle;
                delete ElapsedTime;
                
                // 애니메이션 플래그 업데이트
                if (WeakThis.IsValid())
                {
                    WeakThis->IsAnimationRunning = false;
                    
                    // 계속 애니메이션할지 확인 후 재시작
                    if (WeakThis->IsIndicatorAnimating)
                    {
                        WeakThis->PlayIndicatorAnimation();
                    }
                }
                
                // 여기서 return을 추가하여 아래 코드가 실행되지 않도록 함
                return;
            }
            
            // 표준 애니메이션 완료 (Progress >= 1.0f)
            if (Progress >= 1.0f)
            {
                // 타이머 정리
                if (UWorld* TimerWorld = GEngine && WeakThis.IsValid() ? 
                    GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()) : nullptr)
                {
                    TimerWorld->GetTimerManager().ClearTimer(*AnimHandle);
                }
                
                delete AnimHandle;
                delete ElapsedTime;
                
                // 애니메이션 플래그 업데이트
                if (WeakThis.IsValid())
                {
                    WeakThis->IsAnimationRunning = false;
                    
                    // 계속 애니메이션할지 확인 후 재시작
                    if (WeakThis->IsIndicatorAnimating)
                    {
                        WeakThis->PlayIndicatorAnimation();
                    }
                }
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
            FVector2D TargetPos = {MenuBasePos.X + 42.5f, MenuBasePos.Y - 255.f + 50.f * CurrentMenuIndex};

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