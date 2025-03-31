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
#include "Throw/FruitPhysicsHelper.h"
#include "GameFramework/SpringArmComponent.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;

    // 오빗 기본 값 설정
    CameraOrbitRadius = 130.f;

    CurrentBallType = 1;
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
            UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
        }
        else
        {
            PlateLocation = FVector::ZeroVector;
            UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0) 사용."));
            
            // 카메라 위치 업데이트
            UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
        }
    });

    // 미리보기 공 생성
    CurrentBallType = FMath::RandRange(1, 11);
    UpdatePreviewBall();
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
        
        // 각도 범위 제한
        ThrowAngle = FMath::Clamp(ThrowAngle, 15.0f, 60.0f);
        
        // 모든 시스템에 동일한 각도 전파
        UFruitPhysicsHelper::SetGlobalThrowAngle(ThrowAngle);
        
        UE_LOG(LogTemp, Log, TEXT("각도 조정: 현재 각도 %f"), ThrowAngle);
        
        // 각도 변경 후 미리보기 공 업데이트
        UpdatePreviewBall();
        
        // 각도 변경 후 궤적도 업데이트
        UpdateTrajectory();
    }
}

// 과일 던지기 함수 수정 - 던진 후 즉시 공이 보이도록 수정
void AFruitPlayerController::ThrowFruit()
{
    // 이미 던지는 중이면 무시
    static bool bIsThrowingInProgress = false;
    if (bIsThrowingInProgress)
        return;
    
    // 던지기 시작 표시
    bIsThrowingInProgress = true;
    
    // 미리보기 공 숨기기 - 제거하되 즉시 실제 공 생성
    if (PreviewBall)
    {
        PreviewBall->Destroy();
        PreviewBall = nullptr;
    }
    
    // 입력 비활성화 - 던지는 동안 모든 조작 막기
    DisableInput(this);
    
    // 즉시 공 생성하여 던지기 실행
    UFruitThrowHelper::ThrowFruit(this);
    
    // 0.5초 후 입력 다시 활성화 및 새 미리보기 공 생성
    FTimerHandle ThrowDelayTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ThrowDelayTimerHandle,
        [this]()
        {
            // 던지기 완료 표시
            bIsThrowingInProgress = false;
            
            // 입력 다시 활성화
            EnableInput(this);
            
            // 새로운 미리보기 공 업데이트 (공 타입 바꾸기)
            CurrentBallType = FMath::RandRange(1, 11); // 다음에 던질 공 타입 랜덤 변경
            UpdatePreviewBall();
        },
        BallThrowDelay,
        false // 반복 실행 안 함
    );
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
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
    
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

// 궤적 업데이트 함수 구현
void AFruitPlayerController::UpdateTrajectory()
{
    if (!PreviewBall || !GetWorld())
        return;
    
    // 미리보기 공의 위치와 접시 위치를 이용하여 궤적 업데이트
    FVector StartLocation = PreviewBall->GetActorLocation();
    
    // 접시 위치 가져오기 (저장된 위치 또는 액터 검색)
    FVector TargetLocation = PlateLocation;
    
    // 접시 위치가 초기화되지 않았다면 태그로 검색
    if (TargetLocation.IsZero())
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
        if (PlateActors.Num() > 0)
        {
            TargetLocation = PlateActors[0]->GetActorLocation();
            TargetLocation.Z += 10.0f; // 접시 표면 위로 약간 올림
            PlateLocation = TargetLocation; // 저장
        }
    }
    
    // 유효한 위치가 있는지 확인
    if (TargetLocation.IsZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 위치를 찾을 수 없어 궤적을 업데이트할 수 없습니다."));
        return;
    }
    
    // 로그 출력으로 각도 확인
    UE_LOG(LogTemp, Log, TEXT("궤적 업데이트: 현재 각도=%f, 시작=%s, 목표=%s"), 
        UFruitPhysicsHelper::GetGlobalThrowAngle(), 
        *StartLocation.ToString(), 
        *TargetLocation.ToString());
    
    // 궤적 업데이트 함수 호출
    UFruitTrajectoryHelper::UpdateTrajectoryPath(this, StartLocation, TargetLocation);
}