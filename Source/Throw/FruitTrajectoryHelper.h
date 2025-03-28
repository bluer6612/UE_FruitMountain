#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitTrajectoryHelper.generated.h"

/**
 * 과일 게임에서 포물선 궤적 계산 및 시각화를 담당하는 헬퍼 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitTrajectoryHelper : public UObject
{
    GENERATED_BODY()
    
public:
    // 궤적 관련 새 함수 추가
    UFUNCTION()
    static void CalculateTrajectoryPoints(
        AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        TArray<FVector>& OutTrajectoryPoints);
        
    // 궤적 관련 변수 추가
    UPROPERTY()
    TArray<FVector> TrajectoryPoints;
    
    UPROPERTY()
    int32 TrajectoryID = 0; // 동일 궤적 식별용 ID
    
    UFUNCTION()
    static void UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation);
    
    // 기존 함수명 변경
    UFUNCTION(BlueprintCallable, Category="FruitTrajectory")
    static void DrawTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation);
};