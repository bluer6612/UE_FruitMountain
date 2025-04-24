#include "ComboSystem.h"
#include "Interface/UI/ScoreDisplayWidget.h"
#include "Interface/UI/ScoreWidgetAnimator.h"
#include "Kismet/GameplayStatics.h"

UComboSystem::UComboSystem()
{
    // 기본값 초기화
    ComboCount = 0;
    ComboTimeLimit = 2.0f;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
    CurrentComboScore = 0;
    
    OwnerObject = nullptr;
    ScoreWidgetInstance = nullptr;
}

void UComboSystem::BeginDestroy()
{
    // 데이터 정리
    if (IsRooted())
    {
        RemoveFromRoot();
    }
    
    OnComboScoreFinalized.Clear();
    OnComboEnded.Clear();
    
    Super::BeginDestroy();
}

void UComboSystem::Initialize(UObject* InOwner, UScoreDisplayWidget* InScoreWidget)
{
    OwnerObject = InOwner;
    ScoreWidgetInstance = InScoreWidget;
    
    // 스코어 위젯이 없으면 기본 인스턴스 사용
    if (!ScoreWidgetInstance)
    {
        ScoreWidgetInstance = UScoreDisplayWidget::GetInstance();
    }
}

void UComboSystem::Tick(float DeltaTime)
{
    // 콤보 시간 업데이트
    if (bComboActive)
    {
        ComboRemainingTime -= DeltaTime;
        
        // 콤보 타임 종료
        if (ComboRemainingTime <= 0.0f)
        {
            // 콤보 종료 이벤트 발생
            OnComboEnded.Broadcast(ComboCount);
            
            // 콤보 타이머 만료 처리 함수 호출
            OnComboTimerExpired();
        }
    }
}

int32 UComboSystem::CalculateFinalScore(int32 BallType)
{
    // 1. 기본 점수 계산
    int32 BaseScore = CalculateBaseScore(BallType);
    
    // 2. 콤보 상태 확인 및 업데이트
    if (bComboActive)
    {
        // 기존 콤보 연장
        ComboCount++;
        ExtendComboTime();
    }
    else
    {
        // 새 콤보 시작
        ComboCount = 1;
        bComboActive = true;
        ExtendComboTime();
        CurrentComboScore = 0;
    }
    
    // 3. 연쇄 보너스 계산
    float ComboMultiplier = CalculateComboMultiplier();
    
    // 4. 최종 점수 계산
    int32 FinalScore = FMath::RoundToInt(BaseScore * ComboMultiplier);
    
    // 5. 콤보 점수 누적
    CurrentComboScore += FinalScore;
    
    // 6. UI 업데이트
    DisplayScoreAnimation(FinalScore, ComboCount, ComboMultiplier);
    
    return FinalScore;
}

void UComboSystem::DisplayScoreAnimation(int32 Score, int32 LocalComboCount, float LocalComboMultiplier)
{
    // 위젯 및 애니메이터 유효성 검사
    if (!ScoreWidgetInstance)
    {
        return;
    }
    
    UScoreWidgetAnimator* Animator = ScoreWidgetInstance->GetWidgetAnimator();
    if (!Animator)
    {
        return;
    }
    
    // 애니메이션 콜백 설정 - 새 델리게이트 타입에 맞게 수정
    Animator->OnAnimationComplete.Unbind();
    Animator->OnAnimationComplete.BindUObject(this, &UComboSystem::OnAnimationCompleted);
    
    // 점수 및 콤보 텍스트 애니메이션 실행
    Animator->AnimateScoreText(Score, 0.0f);
    Animator->AnimateComboText(LocalComboCount, LocalComboMultiplier, 0.0f);
    
    // 1초 후 페이드아웃 시작
    Animator->FadeOutBoth(1.0f);
}

void UComboSystem::OnComboTimerExpired()
{
    // 콤보 상태 초기화
    ResetCombo();
    
    // 애니메이션 취소 및 텍스트 숨김
    if (ScoreWidgetInstance && ScoreWidgetInstance->GetWidgetAnimator())
    {
        ScoreWidgetInstance->GetWidgetAnimator()->CancelAnimation();
    }
}

void UComboSystem::OnAnimationCompleted()
{
    // 콤보 점수 최종화
    if (CurrentComboScore > 0)
    {
        OnComboScoreFinalized.Broadcast(CurrentComboScore);
        CurrentComboScore = 0;
    }
}

void UComboSystem::AddToCombo(int32 BallType)
{
    CalculateFinalScore(BallType);
}

void UComboSystem::ResetCombo()
{
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
}

void UComboSystem::ExtendComboTime()
{
    ComboRemainingTime = ComboTimeLimit;
    bComboActive = true;
    
    // 기존 애니메이션 취소
    if (ScoreWidgetInstance && ScoreWidgetInstance->GetWidgetAnimator())
    {
        ScoreWidgetInstance->GetWidgetAnimator()->CancelAnimation();
    }
}

int32 UComboSystem::CalculateBaseScore(int32 BallType) const
{
// 등차수열의 합 공식: n*(n+1)/2
    return (BallType * (BallType + 1)) / 2;
}

float UComboSystem::CalculateComboMultiplier() const
{
    if (ComboCount < 2)
    {
        return 1.0f;
    }
    
// 연쇄 보너스 계산 (2연쇄: 1.1배, 4연쇄: 1.2배, 6연쇄: 1.3배, ...)
    int32 BonusTiers = ComboCount / 2;
    return 1.0f + (BonusTiers * 0.1f);
}

void UComboSystem::SafeCleanup()
{
    // 기존 애니메이션 취소
    if (ScoreWidgetInstance && ScoreWidgetInstance->GetWidgetAnimator())
    {
        ScoreWidgetInstance->GetWidgetAnimator()->CancelAnimation();
    }
    
    // 콤보 상태 초기화
    ResetCombo();
    
    // 델리게이트 정리
    OnComboEnded.Clear();
    OnComboScoreFinalized.Clear();
    OnComboUpdated.Clear();
    
    // 객체 참조 제거
    ScoreWidgetInstance = nullptr;
    OwnerObject = nullptr;
}