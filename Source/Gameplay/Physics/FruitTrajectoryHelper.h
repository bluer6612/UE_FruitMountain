#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitTrajectoryHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitTrajectoryHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 통합된 궤적 업데이트 함수 - 기본 매개변수 추가
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void UpdateTrajectoryPath(class AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent = true, int32 CustomTrajectoryID = 9999);

    // 궤적 시각화 함수 - 공용으로 변경
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static void DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID);

    // 궤적 포인트 계산 함수
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    static TArray<FVector> CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass);

private:
    // 벡터 좌표를 지정된 소수점 자리로 반올림하는 유틸리티 함수
    static FVector RoundVector(const FVector& InVector, int32 DecimalPlaces);

    // FruitTrajectoryHelper.h에 멤버 변수 추가
    static ULineBatchComponent* CustomLineBatcher;
};