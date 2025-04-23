#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboSystem.generated.h"

// 전방 선언
class UScoreDisplayWidget;
class UScoreWidgetAnimator;

// 콤보 관련 델리게이트 정의
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboEndedSignature, int32, FinalComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboScoreFinalizedSignature, int32, FinalComboScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboUpdated, int32, ComboCount, float, ComboMultiplier);

UCLASS()
class UE_FRUITMOUNTAIN_API UComboSystem : public UObject
{
    GENERATED_BODY()

public:
    UComboSystem();
    virtual void BeginDestroy() override;
    
    // 초기화 함수
    void Initialize(UObject* InOwner, UScoreDisplayWidget* InScoreWidget);
    void Tick(float DeltaTime);
    
    // 콤보 관리 함수
    void AddToCombo(int32 BallType);
    void ResetCombo();
    void ExtendComboTime();
    
    // 점수 계산 함수
    UFUNCTION(BlueprintPure, Category = "Combo")
    float CalculateComboMultiplier() const;
    
    UFUNCTION(BlueprintPure, Category = "Combo")
    int32 CalculateBaseScore(int32 BallType) const;
    
    int32 CalculateFinalScore(int32 BallType);
    
    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FComboEndedSignature OnComboEnded;
    
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FComboScoreFinalizedSignature OnComboScoreFinalized;
    
    // OnComboUpdated 델리게이트 멤버
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FOnComboUpdated OnComboUpdated;
    
    UFUNCTION()
    void SafeCleanup();
    
    // 게터 함수
    FORCEINLINE int32 GetComboCount() const { return ComboCount; }
    FORCEINLINE bool IsComboActive() const { return bComboActive; }
    
    // 콤보 설정 함수
    FORCEINLINE void SetComboTimeLimit(float NewLimit)
    {
        ComboTimeLimit = NewLimit;
    }
    
    // 애니메이션 관련 함수
    void DisplayScoreAnimation(int32 Score, int32 ComboCount, float ComboMultiplier);
    
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
    
    // 참조
    UPROPERTY()
    UObject* OwnerObject;
    
    UPROPERTY()
    UScoreDisplayWidget* ScoreWidgetInstance;
    
    // 콜백 함수
    void OnComboTimerExpired();
    void OnAnimationCompleted();
};