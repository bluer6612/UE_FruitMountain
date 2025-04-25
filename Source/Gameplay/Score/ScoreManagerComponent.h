#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScoreManagerComponent.generated.h"

// 전방 선언
class UScoreDisplayWidget;
class UTotalScoreWidget;
class UComboCountWidget;
class UComboSystem;

// ScoreManager 전용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FScoreAddedSignature, int32, Score, int32, ComboCount, float, ComboMultiplier);
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
    
    // 총점 관련 변수
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 TotalScore;
    
    // 콤보 시스템 관련 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float ComboTimeLimit = 1.5f;
    
    // 점수 관련 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FScoreAddedSignature OnScoreAdded;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnComboEndedSignature OnComboEnded;
    
    // 정적 헬퍼 함수 - 게임 내 어디서나 점수 추가 가능
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddScoreStatic(UWorld* World, int32 BallType);
    
    // 정적 헬퍼 함수 - 게임 내 어디서나 콤보 초기화 가능
    UFUNCTION(BlueprintCallable, Category = "Combo")
    static void ResetComboStatic(UWorld* World);
    
    // 점수 추가 함수
    UFUNCTION(BlueprintCallable, Category = "Score")
    int32 AddScore(int32 BallType);
    
    // 총점 관련 함수
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddToTotalScore(int32 ScoreToAdd);
    
    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetTotalScore() const { return TotalScore; }

    // 위젯 인스턴스들
    UPROPERTY()
    UScoreDisplayWidget* ScoreWidgetInstance;

    UPROPERTY()
    UTotalScoreWidget* TotalScoreWidgetInstance;

    UPROPERTY()
    UComboCountWidget* ComboCountWidgetInstance;

    UPROPERTY()
    bool bWidgetCreated;
    
    // 콤보 시스템 접근 함수
    UFUNCTION(BlueprintPure, Category = "Combo")
    UComboSystem* GetComboSystem() const { return ComboSystem; }

    // 콤보 시스템 이벤트 핸들러
    UFUNCTION()
    void OnComboScoreFinalized(int32 FinalComboScore);
    
    UFUNCTION()
    void OnComboUpdated(int32 ComboCount, float ComboMultiplier);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    // 콤보 시스템 인스턴스
    UPROPERTY()
    UComboSystem* ComboSystem;
    
    // 콤보 시스템 초기화 함수
    void InitializeComboSystem();
};