#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitSpawnHelper.generated.h"

/**
 * 과일 게임의 공 스폰 및 위치 계산 관련 공통 함수를 제공하는 헬퍼 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitSpawnHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 공 크기 계산 함수 추가
    UFUNCTION(BlueprintCallable, Category="FruitSpawn")
    static float CalculateBallSize(int32 BallType);
    
    // 공 질량 계산 함수 추가
    UFUNCTION(BlueprintCallable, Category="FruitSpawn")
    static float CalculateBallMass(int32 BallType);
    
    // 공 생성 함수
    UFUNCTION(BlueprintCallable, Category="FruitSpawn")
    static class AActor* SpawnBall(
        class AFruitPlayerController* Controller, 
        const FVector& Location, 
        int32 BallType, 
        bool bEnablePhysics);
    
    // 접시 가장자리 위치 계산 함수
    UFUNCTION(BlueprintCallable, Category="FruitSpawn")
    static FVector CalculatePlateEdgeSpawnPosition(
        class UWorld* World, 
        float HeightOffset = 50.f, 
        float CameraAngle = 0.f);
};