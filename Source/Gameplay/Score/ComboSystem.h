#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboSystem.generated.h"

// 전방 선언
class UScoreDisplayWidget;
class UScoreWidgetAnimator;

// 콤보 관련 델리게이트 정의
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboEndedSignature, int32, FinalComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboUpdatedSignature, int32, ComboCount, float, ComboMultiplier, int32, ComboScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboScoreFinalizedSignature, int32, FinalComboScore);

/**
 * 콤보 시스템 - 콤보 카운트, 타이머, 점수 계산을 관리
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UComboSystem : public UObject
{
    GENERATED_BODY()

public:
    UComboSystem();
    
    // 초기화 함수
    void Initialize(UObject* InOwner, UScoreDisplayWidget* InScoreWidget);
    
    // 틱 업데이트 함수
    void Tick(float DeltaTime);
    
    // 콤보 관리 함수
    void AddToCombo(int32 BallType);
    void ResetCombo();
    void ExtendComboTime();
    
    // 콤보 계산 함수
    UFUNCTION(BlueprintPure, Category = "Combo")
    float CalculateComboMultiplier() const;
    
    UFUNCTION(BlueprintPure, Category = "Combo")
    int32 CalculateBaseScore(int32 BallType) const;
    
    // 점수 계산 및 관리
    int32 CalculateFinalScore(int32 BallType);
    
    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FOnComboEndedSignature OnComboEnded;
    
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FOnComboUpdatedSignature OnComboUpdated;
    
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FOnComboScoreFinalizedSignature OnComboScoreFinalized;
    
    // 게터 함수
    FORCEINLINE int32 GetComboCount() const { return ComboCount; }
    FORCEINLINE bool IsComboActive() const { return bComboActive; }
    FORCEINLINE int32 GetCurrentComboScore() const { return CurrentComboScore; }
    
    // 콤보 설정 함수
    FORCEINLINE void SetComboTimeLimit(float NewLimit) { ComboTimeLimit = NewLimit; }
    
private:
    // 콤보 관련 변수
    UPROPERTY()
    int32 ComboCount;
    
    UPROPERTY()
    float ComboTimeLimit;
    
    UPROPERTY()
    float ComboRemainingTime;
    
    UPROPERTY()
    bool bComboActive;
    
    // 현재 콤보 누적 점수
    UPROPERTY()
    int32 CurrentComboScore;
    
    // 소유 객체 참조
    UPROPERTY()
    UObject* OwnerObject;
    
    // UI 위젯 참조
    UPROPERTY()
    UScoreDisplayWidget* ScoreWidgetInstance;
    
    // 콤보 타이머 만료 처리
    void OnComboTimerExpired();
    
    // 애니메이션 종료 콜백
    UFUNCTION()  // 여기에 반드시 UFUNCTION 매크로가 있어야 함
    void OnScoreAnimationEnded();
    
    // 텍스트 애니메이션 시작
    void StartScoreTextAnimation(int32 Score, int32 ComboCount, float ComboMultiplier);
};