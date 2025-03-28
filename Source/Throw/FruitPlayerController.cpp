#include "FruitPlayerController.h"
#include "Setting/FruitInputMappingManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "Engine/Engine.h"
#include "Actors/CameraOrbitFunctionLibrary.h"
#include "Setting/UE_FruitMountainGameMode.h"
#include "Camera/CameraComponent.h"
#include "Throw/FruitThrowHelper.h"
#include "Throw/FruitTrajectoryHelper.h"
#include "GameFramework/SpringArmComponent.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;

    // 오빗 기본 값 설정
    CameraOrbitAngle = 0.f;
    CameraOrbitRadius = 500.f;
    
    // 카메라 회전 속도 (도/초)
    RotateCameraSpeed = 180.f;

    CurrentBallType = 1;

    // 카메라 높이 설정
    CameraHeightOffset = 50.f;
}

void AFruitPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // GameMode를 캐스팅하여 FruitBallClass 값을 가져옴
    AUE_FruitMountainGameMode* GM = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GM && GM->FruitBallClass)
    {
        FruitBallClass = GM->FruitBallClass;
        UE_LOG(LogTemp, Log, TEXT("PlayerController 쪽 FruitBallClass를 GameMode 쪽 값으로 설정했습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode의 FruitBallClass가 비어 있습니다."));
    }
    
    UE_LOG(LogTemp, Log, TEXT("AFruitPlayerController::BeginPlay 호출됨"));

    // 입력 모드를 게임 전용으로 설정하여 키 입력이 제대로 전달되는지 확인
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    
    UFruitInputMappingManager::ConfigureKeyMappings();

    // 타이머를 사용하여 약간의 지연 후 접시 액터 검색 (타이밍 문제 해결)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        // 접시 액터를 검색하여 회전 기준 위치로 사용
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
        if (PlateActors.Num() > 0)
        {
            PlateLocation = PlateActors[0]->GetActorLocation();
            UE_LOG(LogTemp, Log, TEXT("접시 액터를 찾았습니다: %s"), *PlateLocation.ToString());
            
            // 카메라 위치 업데이트
            UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
        }
        else
        {
            PlateLocation = FVector::ZeroVector;
            UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0) 사용."));
            
            // 카메라 위치 업데이트
            UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
        }
    });

    // 미리보기 공 생성
    CurrentBallType = FMath::RandRange(1, 11);
    UpdatePreviewBall();

    // 카메라 높이 조정
    AdjustCameraHeight();
}

void AFruitPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAxis("AdjustAngle", this, &AFruitPlayerController::AdjustAngle);
        InputComponent->BindAxis("RotateCamera", this, &AFruitPlayerController::RotateCamera);
        InputComponent->BindAction("ThrowFruit", IE_Pressed, this, &AFruitPlayerController::ThrowFruit);
        UE_LOG(LogTemp, Log, TEXT("입력 바인딩 완료"));
    }
}

// 새로운 각도 조정 함수 (축 매핑용)
void AFruitPlayerController::AdjustAngle(float Value)
{
    if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
    {
        // 프레임 시간과 조정 속도를 곱해 부드러운 변화 적용
        float DeltaAngle = Value * AngleAdjustSpeed * GetWorld()->DeltaTimeSeconds;
        ThrowAngle += DeltaAngle;
        
        // 각도 범위 제한 (0도에서 90도 사이)
        ThrowAngle = FMath::Clamp(ThrowAngle, 0.0f, 90.0f);
        
        UE_LOG(LogTemp, Log, TEXT("각도 조정: 현재 각도 %f"), ThrowAngle);
        
        // 각도 변경 후 미리보기 공도 함께 업데이트 (제한적으로 실행됨)
        UpdatePreviewBall();
    }
}

void AFruitPlayerController::ThrowFruit()
{
    UFruitThrowHelper::ThrowFruit(this);
}

// 카메라 회전 처리 함수 수정
void AFruitPlayerController::RotateCamera(float Value)
{
    if (FMath::IsNearlyZero(Value))
        return;
    
    // 회전 속도 적용
    float DeltaAngle = Value * RotateCameraSpeed * GetWorld()->GetDeltaSeconds();
    
    // 각도 업데이트
    CameraOrbitAngle += DeltaAngle;
    
    // 360도 범위 내로 제한
    CameraOrbitAngle = FMath::Fmod(CameraOrbitAngle, 360.0f);
    if (CameraOrbitAngle < 0.0f)
        CameraOrbitAngle += 360.0f;
    
    // 중요: 카메라 위치 업데이트
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
    
    // 즉시 공 위치 업데이트 (재귀 호출 없이)
    UFruitThrowHelper::UpdatePreviewBall(this);
}

// 실제 업데이트 수행 함수 수정
void AFruitPlayerController::ExecutePreviewBallUpdate()
{
    bPreviewBallUpdatePending = false;
    
    // 공 위치 업데이트 함수 직접 호출 (UFruitThrowHelper를 통해)
    UFruitThrowHelper::UpdatePreviewBall(this);
}

// UpdatePreviewBall 함수 수정 - 무한 재귀 방지
void AFruitPlayerController::UpdatePreviewBall()
{
    // 이미 업데이트가 예약되어 있으면 중복 실행하지 않음
    if (!bPreviewBallUpdatePending)
    {
        bPreviewBallUpdatePending = true;
        
        // 타이머 설정: 일정 시간 후에만 업데이트 실행
        GetWorld()->GetTimerManager().SetTimer(
            PreviewBallUpdateTimerHandle,
            this,
            &AFruitPlayerController::ExecutePreviewBallUpdate,
            PreviewBallUpdateDelay,
            false // 반복 실행 안 함
        );
    }
}

// 카메라 높이 조정 함수 추가
void AFruitPlayerController::AdjustCameraHeight()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;
    
    // 카메라 컴포넌트 찾기
    UCameraComponent* CameraComp = Cast<UCameraComponent>(
        ControlledPawn->GetComponentByClass(UCameraComponent::StaticClass()));
    
    if (CameraComp)
    {
        // 현재 카메라 상대 위치 가져오기
        FVector CameraRelativeLocation = CameraComp->GetRelativeLocation();
        
        // Z축 높이 50 유닛 증가
        CameraRelativeLocation.Z += CameraHeightOffset;
        
        // 새 위치 적용
        CameraComp->SetRelativeLocation(CameraRelativeLocation);
        
        UE_LOG(LogTemp, Warning, TEXT("카메라 높이 조정: %f 유닛 증가"), CameraHeightOffset);
    }
    else
    {
        // 스프링암 컴포넌트 찾기 (카메라가 스프링암에 부착된 경우)
        USpringArmComponent* SpringArm = Cast<USpringArmComponent>(
            ControlledPawn->GetComponentByClass(USpringArmComponent::StaticClass()));
            
        if (SpringArm)
        {
            // 스프링암 상대 위치 가져오기
            FVector SpringArmRelativeLocation = SpringArm->GetRelativeLocation();
            
            // Z축 높이 50 유닛 증가
            SpringArmRelativeLocation.Z += CameraHeightOffset;
            
            // 새 위치 적용
            SpringArm->SetRelativeLocation(SpringArmRelativeLocation);
            
            UE_LOG(LogTemp, Warning, TEXT("스프링암 높이 조정: %f 유닛 증가"), CameraHeightOffset);
        }
    }
}