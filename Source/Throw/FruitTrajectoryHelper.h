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
    
    // 궤적 시각화 함수
    UFUNCTION()
    static void DrawTrajectoryPath(
        UWorld* World,
        const TArray<FVector>& TrajectoryPoints,
        const FVector& TargetLocation,
        bool bPersistent,
        int32 TrajectoryID);
    
    // 통합된 궤적 업데이트 함수 - 기본 매개변수 추가
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void UpdateTrajectoryPath(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        bool bPersistent = true, 
        int32 CustomTrajectoryID = 9999);
    
    // 하위 호환성을 위한 영구 궤적 그리기 함수
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void DrawPersistentTrajectoryPath(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation);
};