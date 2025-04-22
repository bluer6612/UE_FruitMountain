#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboSystem.generated.h"

// 전방 선언
class UScoreDisplayWidget;
class UScoreWidgetAnimator;

// 콤보 관련 델리게이트 정의 - 네임스페이스 활용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboEndedSignature, int32, FinalComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComboUpdatedSignature, int32, ComboCount, float, ComboMultiplier, int32, ComboScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboScoreFinalizedSignature, int32, FinalComboScore);

UCLASS()
class UE_FRUITMOUNTAIN_API UComboSystem : public UObject
{
    GENERATED_BODY()

public:
    UComboSystem();
    
    // BeginDestroy 오버라이드 추가
    virtual void BeginDestroy() override;
    
    // 명시적인 정리 함수 추가
    UFUNCTION()
    void SafeCleanup();
    
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
    
    // 델리게이트 - 이름 변경하여 중복 방지
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FComboEndedSignature OnComboEnded;
    
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FComboUpdatedSignature OnComboUpdated;
    
    UPROPERTY(BlueprintAssignable, Category = "Combo")
    FComboScoreFinalizedSignature OnComboScoreFinalized;
    
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
    UFUNCTION()
    void OnScoreAnimationEnded();
    
    // 텍스트 애니메이션 시작
    void StartScoreTextAnimation(int32 Score, int32 ComboCount, float ComboMultiplier);
};