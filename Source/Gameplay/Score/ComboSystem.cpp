#include "ComboSystem.h"
#include "Interface/UI/ScoreDisplayWidget.h"
#include "Interface/UI/ScoreWidgetAnimator.h"
#include "Kismet/GameplayStatics.h"

UComboSystem::UComboSystem()
{
    // 기본값 초기화
    ComboCount = 0;
    ComboTimeLimit = 1.5f;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
    CurrentComboScore = 0;
    
    OwnerObject = nullptr;
    ScoreWidgetInstance = nullptr;
}

void UComboSystem::Initialize(UObject* InOwner, UScoreDisplayWidget* InScoreWidget)
{
    OwnerObject = InOwner;
    ScoreWidgetInstance = InScoreWidget;
    
    // 스코어 위젯이 없으면 기본 인스턴스 사용
    if (!ScoreWidgetInstance)
    {
        ScoreWidgetInstance = UScoreDisplayWidget::Instance;
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
            UE_LOG(LogTemp, Display, TEXT("콤보 시간 종료! 최종 콤보 카운트: %d"), ComboCount);
            
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
    bool bIsNewCombo = !bComboActive;
    
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
        // 새 콤보 시작 시 콤보 점수 초기화
        CurrentComboScore = 0;
    }
    
    // 3. 연쇄 보너스 계산
    float ComboMultiplier = CalculateComboMultiplier();
    
    // 4. 최종 점수 계산
    int32 FinalScore = FMath::RoundToInt(BaseScore * ComboMultiplier);
    
    // 5. 콤보 점수 누적
    CurrentComboScore += FinalScore;
    
    // 6. 로그 출력
    if (ComboCount >= 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("%d연쇄 병합! 기본점수: %d, 보너스율: %.1f배, 최종점수: %d"),
               ComboCount, BaseScore, ComboMultiplier, FinalScore);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("과일 병합 점수: %d (레벨 %d)"), FinalScore, BallType);
    }
    
    // 7. 콤보 업데이트 이벤트 발생
    OnComboUpdated.Broadcast(ComboCount, ComboMultiplier, CurrentComboScore);
    
    // 8. UI 업데이트 - 텍스트 애니메이션
    StartScoreTextAnimation(FinalScore, ComboCount, ComboMultiplier);
    
    return FinalScore;
}

void UComboSystem::StartScoreTextAnimation(int32 Score, int32 ComboCount, float ComboMultiplier)
{
    if (!ScoreWidgetInstance)
    {
        return;
    }
    
    // 스코어 위젯에 점수 표시 요청
    ScoreWidgetInstance->DisplayScoreGain(Score, ComboCount, ComboMultiplier);
    
    // 애니메이션 종료 시 콜백 등록
    UScoreWidgetAnimator* Animator = ScoreWidgetInstance->GetWidgetAnimator();
    if (Animator)
    {
        Animator->OnAnimationEnd.Clear(); // 기존 연결 제거
        Animator->OnAnimationEnd.AddDynamic(this, &UComboSystem::OnScoreAnimationEnded);
    }
}

void UComboSystem::OnComboTimerExpired()
{
    // ScoreWidgetInstance의 애니메이션 종료 시 총점 반영
    if (ScoreWidgetInstance)
    {
        ScoreWidgetInstance->ResetComboDisplay();
        
        // 애니메이션은 이미 진행 중이므로 추가 작업 없음
        // 애니메이션 종료 시 OnScoreAnimationEnded에서 처리
    }
    else if (UScoreDisplayWidget::Instance)
    {
        UScoreDisplayWidget::Instance->ResetComboDisplay();
    }
    
    // 콤보 상태 초기화는 유지 (콤보 점수 초기화는 아직 하지 않음)
    ResetCombo();
}

void UComboSystem::OnScoreAnimationEnded()
{
    // 콤보 점수를 최종화하고 점수 이벤트 발생
    if (CurrentComboScore > 0)
    {
        // 콤보 점수 최종화 이벤트 발생
        OnComboScoreFinalized.Broadcast(CurrentComboScore);
        
        // 로그 출력
        UE_LOG(LogTemp, Display, TEXT("콤보 점수 최종화: %d"), CurrentComboScore);
        
        // 콤보 점수 초기화
        CurrentComboScore = 0;
    }
}

void UComboSystem::AddToCombo(int32 BallType)
{
    // 점수 계산 및 콤보 업데이트
    int32 FinalScore = CalculateFinalScore(BallType);
}

void UComboSystem::ResetCombo()
{
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
    // CurrentComboScore는 초기화하지 않음 - 애니메이션 종료 후 초기화
}

void UComboSystem::ExtendComboTime()
{
    ComboRemainingTime = ComboTimeLimit;
    bComboActive = true;
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