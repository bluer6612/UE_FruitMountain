#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FruitPlayerController.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFruitPlayerController();

    virtual void BeginPlay() override;

    // 던질 공(과일) 액터의 클래스, 에디터에서 지정 (예: 블루프린트 클래스)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwing")
    TSubclassOf<AActor> FruitBallClass;

    // 현재 포물선 발사 각도 (피칭 각도)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwing")
    float ThrowAngle;

    // 포물선 발사 힘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwing")
    float ThrowForce;

    // 각도 조정 단위 (한 번 입력 시 변경되는 값)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwing")
    float AngleStep;

    // 각도 조정 속도 (도/초)
    UPROPERTY(EditAnywhere, Category = "Throwing")
    float AngleAdjustSpeed = 60.f;
    
    // 미리보기 공 액터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
    AActor* PreviewBall;

    // 현재 선택된 공 타입 (1~11)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
    int32 CurrentBallType;
    
    // 미리보기 공 업데이트 함수
    void UpdatePreviewBall();

protected:
    virtual void SetupInputComponent() override;

    // 프로젝트 시작 시 C++로 키 매핑 설정
    void SetupProjectInputMappings();

    // 스페이스바 입력에 따라 과일을 던짐
    void ThrowFruit();

    // Axis 입력 처리 함수
    void AdjustAngle(float Value);

    // Axis 입력으로 카메라 회전 처리 함수
    void RotateCamera(float Value);

    // 카메라(Pawn) 오빗 관련 변수 및 함수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float CameraOrbitAngle; // 현재 각도 (도 단위)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float CameraOrbitRadius; // 접시와의 거리

    // 카메라 회전 속도 (도/초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float RotateCameraSpeed;

    // 회전 기준이 되는 접시(Plate)의 위치
    FVector PlateLocation;

private:
    // 미리보기 공 업데이트 제한을 위한 변수들
    FTimerHandle PreviewBallUpdateTimerHandle;
    bool bPreviewBallUpdatePending = false;
    const float PreviewBallUpdateDelay = 0.02f;
    
    // 실제 업데이트 수행 함수
    void ExecutePreviewBallUpdate();
};