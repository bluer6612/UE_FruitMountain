#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitTrajectoryHelper.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitTrajectoryHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 궤적 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    static void UpdateTrajectoryPath(class AFruitPlayerController* Controller, const FVector& StartLocation, bool bPersistent = true, int32 CustomTrajectoryID = 9999);

    // 궤적 시각화 함수 - 공용으로 변경
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    static void DrawTrajectoryPath(UWorld* World, const TArray<FVector>& Points, int32 TrajectoryID);

    // 궤적 포인트 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Trajectory")
    static TArray<FVector> CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass);

    // 궤적 시각화 시스템 초기화 함수 추가
    static void ResetTrajectorySystem();

private:
    // 벡터 좌표를 지정된 소수점 자리로 반올림하는 유틸리티 함수
    static FVector RoundVector(const FVector& InVector, int32 DecimalPlaces);

    // 언리얼 엔진에서 정적 변수는 PIE 모드 간에 공유, "Object is not in global object array"
    // 오류는 주로 이전 PIE 세션의 객체 참조 (ULineBatchComponent라는 정적 변수)
    // 앞으로 정적 변수는 명시적 초기화/정리 패턴을 적용
    static ULineBatchComponent* CustomLineBatcher;
};