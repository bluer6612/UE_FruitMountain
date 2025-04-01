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

private:
    // 베지어 곡선으로 포물선 포인트 계산
    static TArray<FVector> CalculateBezierPoints(
        const FVector& Start, 
        const FVector& End, 
        float PeakHeight, 
        int32 PointCount);

    // 궤적 시각화 함수
    UFUNCTION()
    static void DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID);
};