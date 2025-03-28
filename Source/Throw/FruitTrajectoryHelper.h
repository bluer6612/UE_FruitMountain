#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitTrajectoryHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitTrajectoryHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 궤적 계산 공통 함수
    UFUNCTION()
    static TArray<FVector> CalculateTrajectoryPoints(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        float BallMass);
    
    // 궤적 시각화 공통 함수
    UFUNCTION()
    static void DrawTrajectoryPath(
        UWorld* World,
        const TArray<FVector>& TrajectoryPoints,
        const FVector& TargetLocation,
        bool bPersistent,
        int32 TrajectoryID);
    
    // 다음 두 함수는 기존과 동일한 이름으로 유지 (호환성)
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void UpdateTrajectoryPath(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation);
    
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void DrawPersistentTrajectoryPath(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation);
};