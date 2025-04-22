#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeFeedbackHelper.generated.h"

class UPrimitiveComponent;
class AFruitBall;
class APlateActor;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeFeedbackHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 여러 과일 안정화 처리 (단일 과일도 처리 가능)
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void StabilizeFruits(UWorld* World, float DampingMultiplier = 3.0f, bool bIsNewFruit = false);
    
    // 단일 과일 안정화 - 모든 충돌 유형에 대해 통합된 로직
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void StabilizeSingleFruit(AFruitBall* Fruit, float DampingMultiplier = 3.0f, 
                                    bool bIsNewFruit = false);

    // 병합 이펙트 재생
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
};