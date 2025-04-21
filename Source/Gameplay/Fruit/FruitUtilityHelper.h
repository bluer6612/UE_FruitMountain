#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitUtilityHelper.generated.h"

// 전방 선언
class UScoreManagerComponent;
class AFruitBall;
class UWorld;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitUtilityHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 점수 추가
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddScore(UWorld* World, int32 BallType);
    
    // 병합 이펙트 재생
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    
    // 모든 과일 또는 단일 과일 안정화 (통합된 함수)
    UFUNCTION(BlueprintCallable, Category = "Fruit|Physics")
    static void StabilizeFruits(UWorld* World, AFruitBall* SingleFruit = nullptr, float DampingMultiplier = 20.0f, bool bIsNewFruit = false);
    
    // 단일 과일 물리 속성 안정화 (내부 헬퍼 함수)
    static void StabilizeSingleFruit(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit);
    
    // 연쇄 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void ResetCombo(UWorld* World);

    // 모든 과일 메시를 미리 로드
    UFUNCTION(BlueprintCallable, Category = "Fruit")
    static void PreloadAllFruitMeshes(UWorld* World);
};