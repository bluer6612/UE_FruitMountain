#include "ScoreManagerComponent.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Kismet/GameplayStatics.h"

UScoreManagerComponent::UScoreManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본값 초기화
    CurrentScore = 0;
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
}

void UScoreManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UScoreManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
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
            
            // 콤보 초기화
            ResetCombo();
        }
    }
}

int32 UScoreManagerComponent::AddScore(int32 BallType)
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
    }
    
    // 3. 연쇄 보너스 계산
    float ComboMultiplier = CalculateComboMultiplier();
    
    // 4. 최종 점수 계산
    int32 FinalScore = FMath::RoundToInt(BaseScore * ComboMultiplier);
    
    // 5. 점수 추가
    CurrentScore += FinalScore;
    
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
    
    // 7. 이벤트 발생
    OnScoreAdded.Broadcast(FinalScore, ComboCount, ComboMultiplier);
    
    // 8. 게임모드에 점수 업데이트 (필요시)
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        // GameMode->UpdateScore(CurrentScore);
    }
    
    return FinalScore;
}

void UScoreManagerComponent::ResetCombo()
{
    ComboCount = 0;
    ComboRemainingTime = 0.0f;
    bComboActive = false;
}

void UScoreManagerComponent::ExtendComboTime()
{
    ComboRemainingTime = ComboTimeLimit;
    bComboActive = true;
}

int32 UScoreManagerComponent::CalculateBaseScore(int32 BallType) const
{
    // 등차수열의 합 공식: n*(n+1)/2
    return (BallType * (BallType + 1)) / 2;
}

float UScoreManagerComponent::CalculateComboMultiplier() const
{
    if (ComboCount < 2)
        return 1.0f;
    
    // 연쇄 보너스 계산 (2연쇄: 1.1배, 4연쇄: 1.2배, 6연쇄: 1.3배, ...)
    int32 BonusTiers = ComboCount / 2;
    return 1.0f + (BonusTiers * 0.1f);
}