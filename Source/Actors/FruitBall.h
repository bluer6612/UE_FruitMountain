#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FruitBall.generated.h"

UCLASS()
class AFruitBall : public AActor
{
    GENERATED_BODY()
    
public:    
    AFruitBall();
    
    // 공의 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UStaticMeshComponent* MeshComponent;
    
    // 기본 공 크기 (월드 스케일)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ball Properties")
    float BaseBallSize = 15.0f;
    
    // 밀도 계수 - 다른 클래스에서 참조할 수 있도록 상수로 정의
    static constexpr float DensityFactor = 7.5f; // 추가로 5배 증가된 밀도 (1.5f에서 7.5f로)
    
    // 공 크기 getter 함수 - 외부에서 공 크기 접근용
    UFUNCTION(BlueprintCallable, Category="Ball Properties")
    float GetBaseBallSize() const { return BaseBallSize; }
    
    // 공 타입에 따른 크기 계산 (10%씩 증가)
    UFUNCTION(BlueprintCallable, Category="Ball Properties")
    static float CalculateBallSize(int32 BallType, float BaseBallScale = 15.0f);
    
    // 공 타입과 크기에 따른 질량 계산
    UFUNCTION(BlueprintCallable, Category="Ball Properties")
    static float CalculateBallMass(int32 BallType, float BaseBallScale = 15.0f);
};