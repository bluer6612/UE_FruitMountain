#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FruitBall.generated.h"

UCLASS()
// 주석: 공(FruitBall) 액터 클래스 - 물리 시뮬레이션을 사용
class UE_FRUITMOUNTAIN_API AFruitBall : public AActor
{
    GENERATED_BODY()

public:
    AFruitBall();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 공의 메시 컴포넌트 (물리 시뮬레이션 활성화)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;
};