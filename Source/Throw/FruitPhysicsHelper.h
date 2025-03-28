#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitPhysicsHelper.generated.h"

UCLASS()
class UFruitPhysicsHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 던지기 파라미터 계산 함수
    UFUNCTION(BlueprintCallable, Category="Physics")
    static void CalculateThrowParameters(
        class AFruitPlayerController* Controller,
        const FVector& StartLocation,
        const FVector& TargetLocation,
        float& OutAdjustedForce,
        FVector& OutLaunchDirection,
        float BallMass);

    // 전역 던지기 각도 설정 함수 추가
    UFUNCTION(BlueprintCallable, Category="Physics")
    static void SetGlobalThrowAngle(float Angle);

    // 현재 설정된 전역 던지기 각도 반환
    UFUNCTION(BlueprintCallable, Category="Physics")
    static float GetGlobalThrowAngle();

    // 던지기 각도 저장용 정적 변수
    static float GlobalThrowAngle;
};