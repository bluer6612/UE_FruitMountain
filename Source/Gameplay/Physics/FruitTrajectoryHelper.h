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
    
    // 통합된 궤적 업데이트 함수 - 기본 매개변수 추가
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void UpdateTrajectoryPath(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        bool bPersistent = true, 
        int32 CustomTrajectoryID = 9999);

    // 궤적 시각화 함수 - 공용으로 변경
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID);
};