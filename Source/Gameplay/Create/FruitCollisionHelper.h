#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitCollisionHelper.generated.h"

class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitCollisionHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 충돌 이벤트 관리
    UFUNCTION(BlueprintCallable, Category = "Collision")
    static void RegisterCollisionHandlers(AFruitBall* Fruit);
    
    // 충돌 처리 함수
    UFUNCTION(BlueprintCallable, Category = "Collision")
    static void HandleFruitCollision(AFruitBall* FruitA, AFruitBall* FruitB, const FHitResult& Hit);
};