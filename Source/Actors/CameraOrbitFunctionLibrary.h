#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CameraOrbitFunctionLibrary.generated.h"

/**
 * 주석: 접시를 기준으로 카메라 오빗 위치를 업데이트하는 함수 라이브러리
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UCameraOrbitFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ControlledPawn: 소유 Pawn, PlateLocation: 접시 위치,
    // OrbitAngle: 현재 오빗 각도 (도 단위), OrbitRadius: 접시와의 거리,
    // ElevationAngle: 접시를 내려다보는 각도 (기본 30도)
    UFUNCTION(BlueprintCallable, Category = "Camera Orbit")
    static void UpdateCameraOrbit(APawn* ControlledPawn, const FVector& PlateLocation, float OrbitAngle, float OrbitRadius, float ElevationAngle = 30.f);
};