#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreAddedSignature, int32, Score, int32, ComboCount, float, ComboMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboEndedSignature, int32, FinalComboCount);

UCLASS(ClassGroup=(Gameplay), meta=(BlueprintSpawnableComponent))
class UE_FRUITMOUNTAIN_API UScoreManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UScoreManagerComponent();
    
    // 게임 점수 관련 변수
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 CurrentScore;
    
    // 콤보 관련 변수
    UPROPERTY(BlueprintReadOnly, Category = "Combo")
    int32 ComboCount;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float ComboTimeLimit = 1.5f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Combo")
    float ComboRemainingTime;
    
    UPROPERTY(BlueprintReadOnly, Category = "Combo")
    bool bComboActive;
    
    // 점수 관련 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreAddedSignature OnScoreAdded;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnComboEndedSignature OnComboEnded;
    
    // 점수 추가 함수
    UFUNCTION(BlueprintCallable, Category = "Score")
    int32 AddScore(int32 BallType);
    
    // 콤보 관리 함수
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ResetCombo();
    
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ExtendComboTime();
    
    UFUNCTION(BlueprintPure, Category = "Score")
    int32 CalculateBaseScore(int32 BallType) const;
    
    UFUNCTION(BlueprintPure, Category = "Score")
    float CalculateComboMultiplier() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};