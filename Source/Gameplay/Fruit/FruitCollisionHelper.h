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
    
    // 에디터 플레이 시작/종료 시 호출할 초기화 함수
    UFUNCTION()
    static void ResetCollisionSystem();
    
private:
    // 등록된 과일 추적용 정적 배열
    static TArray<TWeakObjectPtr<AFruitBall>> RegisteredFruits;
};