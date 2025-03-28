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
    // 미리보기 공 업데이트 기능
    UFUNCTION(BlueprintCallable, Category="FruitTrajectory")
    static void UpdatePreviewBall(class AFruitPlayerController* Controller);

    // 공 궤적 계산 및 표시 함수
    UFUNCTION(BlueprintCallable, Category="FruitTrajectory")
    static void DrawTrajectoryPath(class AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation);
};