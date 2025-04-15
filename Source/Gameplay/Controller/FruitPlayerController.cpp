#include "FruitPlayerController.h"
#include "System/Input/FruitInputMappingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "System/Camera/CameraOrbitFunctionLibrary.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Gameplay/Physics/FruitThrowHelper.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"
#include "Gameplay/Physics/FruitPhysicsHelper.h"
#include "Actors/FruitBall.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;

    CameraOrbitRadius = 110.f;

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

    // 입력 매핑 설정
    UFruitInputMappingManager::ConfigureKeyMappings();
    SetInputMode(FInputModeGameAndUI());
    SetShowMouseCursor(true);

    // 레벨 로딩이 완전히 완료된 후 호출
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &AFruitPlayerController::OnPostLevelLoadComplete);

    // 또는 안전하게 지연시키기
    GetWorld()->GetTimerManager().SetTimer(
        FirstPreviewBallTimerHandle,
        this,
        &AFruitPlayerController::OnPostLevelLoadComplete,
        0.5f, // 충분한 지연 시간
        false
    );

    UE_LOG(LogTemp, Log, TEXT("AFruitPlayerController::BeginPlay 호출 완료됨"));
}

// 레벨 로딩 완료 후 초기화 함수
void AFruitPlayerController::OnPostLevelLoadComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("레벨 로딩 완료 - 안전한 객체 생성 시작"));

    // 접시 찾기 등 초기화 작업
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // 접시 경계 구하기 (중심점 정확히 계산)
        FVector PlateOrigin;
        FVector PlateExtent;
        PlateActors[0]->GetActorBounds(false, PlateOrigin, PlateExtent);
        PlateOrigin.Z += 10.0f; // 접시 표면 위로 약간 올림

        // 직접 중심점 설정 (오프셋 없이)
        PlateLocation = PlateOrigin;

        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾았습니다: %s"), *PlateLocation.ToString());

        // 카메라 설정
        UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);

        // 이제 안전하게 첫번째 미리보기 과일 생성
        CurrentBallType = FMath::RandRange(1, AFruitBall::MaxBallType);

        // 특수 안전 모드로 첫 미리보기 공 생성 (직접 로직 구현)
        SafeCreateFirstPreviewBall();
    }
    else
    {
        PlateLocation = FVector::ZeroVector;
        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
        return;
    }
}

// 첫 번째 미리보기 공 안전하게 생성
void AFruitPlayerController::SafeCreateFirstPreviewBall()
{
    UE_LOG(LogTemp, Warning, TEXT("첫번째 미리보기 과일 안전 생성 시도"));

    // 이전 PreviewBall이 있으면 완전히 제거
    if (PreviewBall)
    {
        if (IsValid(PreviewBall))
        {
            PreviewBall->Destroy();
        }
        PreviewBall = nullptr;
    }

    // 엔진 GC 실행 유도 (메모리 정리)
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

    // 약간의 추가 지연 후 실제 생성 (핵심!)
    FTimerHandle SafeSpawnTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        SafeSpawnTimerHandle,
        [this]()
        {
            if (IsValid(this) && IsValid(GetWorld()))
            {
                // 수동으로 안전하게 공 스폰
                UE_LOG(LogTemp, Warning, TEXT("안전 모드로 첫번째 미리보기 과일 생성"));

                // 기본 값으로 공 생성 요청
                bPreviewBallUpdatePending = false; // 플래그 초기화 중요
                UFruitThrowHelper::UpdatePreviewBall(this, false);

                // 공 생성 확인 후 회전 적용
                if (PreviewBall && IsValid(PreviewBall))
                {
                    UE_LOG(LogTemp, Warning, TEXT("첫번째 미리보기 과일 생성 성공"));
                    SetFruitRotation(PreviewBall);

                    // 궤적 업데이트
                    if (PreviewBall->GetActorLocation() != FVector::ZeroVector)
                    {
                        UFruitTrajectoryHelper::UpdateTrajectoryPath(this, PreviewBall->GetActorLocation());
                    }
                }
            }
        },
        0.2f, // 추가 지연
        false
    );
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

// 과일 던지기 함수 수정
void AFruitPlayerController::ThrowFruit()
{
    // 이미 던지는 중이면 무시 - static 변수 대신 멤버 변수 사용
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
    
    // 일정 딜레이 후 입력 다시 활성화 및 새 미리보기 공 생성
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
            CurrentBallType = FMath::RandRange(1, AFruitBall::MaxBallType); // 다음에 던질 공 타입 랜덤 변경
            UpdatePreviewBallWithDebounce();
        },
        BallThrowDelay,
        false // 반복 실행 안 함
    );
}

// 새로운 각도 조정 함수 (축 매핑용)
void AFruitPlayerController::AdjustAngle(float Value)
{
    if (FMath::Abs(Value) < KINDA_SMALL_NUMBER)
        return;
    
    // 프레임 시간과 조정 속도를 곱해 부드러운 변화 적용
    float DeltaAngle = Value * AngleAdjustSpeed * GetWorld()->DeltaTimeSeconds;
    
    // 변경 전에 최종 각도가 제한 범위를 벗어나는지 확인 (한 번의 if문으로 처리)
    float NewAngle = ThrowAngle + DeltaAngle;
    
    // 변경 전에 제한 검사 - 범위를 벗어나면 작업 수행하지 않음
    if ((NewAngle > UFruitPhysicsHelper::MaxThrowAngle && DeltaAngle > 0.0f) || 
        (NewAngle < UFruitPhysicsHelper::MinThrowAngle && DeltaAngle < 0.0f))
    {
        return;
    }
    
    // 허용 범위 내의 변경이면 실행
    ThrowAngle = NewAngle;
    
    // 안전을 위한 클램핑 (불필요하지만 추가 보호)
    ThrowAngle = FMath::Clamp(ThrowAngle, UFruitPhysicsHelper::MinThrowAngle, UFruitPhysicsHelper::MaxThrowAngle);
    
    // 각도 변경 후 미리보기 공 및 궤적 업데이트
    UpdatePreviewBallWithDebounce();
}

// 카메라 회전 처리 함수 - 과일 회전 보정 수정
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
    
    // 카메라 위치 업데이트
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius);
    
    // 위치만 업데이트하고 회전은 별도로 처리
    UFruitThrowHelper::UpdatePreviewBall(this, false);
    
    // 과일이 항상 카메라를 바라보도록 회전 조정
    if (PreviewBall)
    {
        SetFruitRotation(PreviewBall);
    }
}

// 실제 업데이트 수행 함수 (무한 루프 방지 용으로 이중으로 거침)
void AFruitPlayerController::ExecutePreviewBallUpdate()
{
    bPreviewBallUpdatePending = false;
    
    // 공 위치 업데이트 (회전은 별도 처리)
    UFruitThrowHelper::UpdatePreviewBall(this, false);
    
    // 과일 각도 업데이트
    if (PreviewBall)
    {
        SetFruitRotation(PreviewBall);
    }
}

// 미리보기 공 업데이트 함수 - 연속 호출 방지 (디바운싱) 처리
void AFruitPlayerController::UpdatePreviewBallWithDebounce()
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

// 과일 회전 설정 함수 구현 - 항상 카메라 각도 고려
void AFruitPlayerController::SetFruitRotation(AActor* Fruit)
{
    if (!Fruit)
    {
        return;
    }
    
    // 던지기 각도에 따른 피치 회전 계산
    float PitchAngle = ThrowAngle - 30.0f; // 30도라는 적절한 각도값 찾아 놓았음
    
    // 요(Yaw) 회전 - 항상 카메라가 바라보는 방향을 보도록 조정
    float YawAngle = CameraOrbitAngle - 180.0f;
    
    // 360도 범위 내로 제한
    YawAngle = FMath::Fmod(YawAngle, 360.0f);
    if (YawAngle < 0.0f)
        YawAngle += 360.0f;
    
    // 새로운 회전 설정
    FRotator NewRotation = FRotator(PitchAngle, YawAngle, 0.0f);
    Fruit->SetActorRotation(NewRotation);
}