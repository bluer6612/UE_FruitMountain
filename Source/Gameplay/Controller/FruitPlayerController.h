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

    // 각도 조정 속도 (도/초)
    UPROPERTY(EditAnywhere, Category = "Throwing")
    float AngleAdjustSpeed = 30.f;

    // 공 던지기 관련 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwing")
    float BallThrowDelay = 1.f;

    // 카메라 회전 속도 (도/초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float RotateCameraSpeed = 180.f;

    // 카메라(Pawn) 오빗 관련 변수 및 함수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float CameraOrbitAngle = 0.f; // 현재 각도 (도 단위)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Orbit")
    float CameraOrbitRadius; // 접시와의 거리
    
    // 미리보기 공 액터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
    AActor* PreviewBall;

    // 현재 선택된 공 타입 (1~11)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
    int32 CurrentBallType;

    // 던지기 상태 추적을 위한 변수 (static 대신 멤버 변수로)
    UPROPERTY()
    bool bIsThrowingInProgress = false;

    // 접시 위치 저장 변수 (모든 클래스에서 공유)
    UPROPERTY(BlueprintReadOnly, Category="Plate")
    FVector PlateLocation;

    // 미리보기 공 업데이트 함수
    void UpdatePreviewBall();

    // 궤적 업데이트 함수 추가
    UFUNCTION(BlueprintCallable, Category="Trajectory")
    void UpdateTrajectory();
    
    // 과일 회전 설정 기본값 - 회전 각도 계산에 사용
    static constexpr float FruitPitchAngleOffset = 30.0f;
    
    // 과일 회전 설정 함수 - 던지기 각도와 카메라 각도 기반
    void SetFruitRotation(AActor* Fruit, bool bConsiderCameraAngle = true);

private:
    // 미리보기 공 업데이트 제한을 위한 변수들
    FTimerHandle PreviewBallUpdateTimerHandle;
    bool bPreviewBallUpdatePending = false;
    const float PreviewBallUpdateDelay = 0.02f;

    virtual void SetupInputComponent() override;
    
    // 실제 업데이트 수행 함수
    void ExecutePreviewBallUpdate();

    // 스페이스바 입력에 따라 과일을 던짐
    void ThrowFruit();

    // Axis 입력 처리 함수
    void AdjustAngle(float Value);

    // Axis 입력으로 카메라 회전 처리 함수
    void RotateCamera(float Value);
};