#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitCollisionHelper.generated.h"

class AFruitBall;
class UPrimitiveComponent;

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
    
    // 충돌 이벤트 처리 함수 (OnBallHit 이동)
    UFUNCTION(BlueprintCallable, Category = "Collision")
    static void HandleBallHit(AFruitBall* Fruit, UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    
    // 접시 위 안정화 처리 (StabilizeOnPlate 이동)
    UFUNCTION(BlueprintCallable, Category = "Collision")
    static void StabilizeOnPlate(AFruitBall* Fruit, UPrimitiveComponent* HitComponent);
};