#include "FruitPlayerController.h"
#include "Manager/FruitInputMappingManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/InputSettings.h"
#include "Engine/Engine.h"
#include "Actors/CameraOrbitFunctionLibrary.h"

AFruitPlayerController::AFruitPlayerController()
{
    ThrowAngle = 45.f;
    ThrowForce = 1000.f;
    AngleStep = 5.f;

    // 오빗 기본 값 설정
    CameraOrbitAngle = 0.f;
    CameraOrbitRadius = 500.f;
}

void AFruitPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("AFruitPlayerController::BeginPlay 호출됨"));

    // 입력 모드를 게임 전용으로 설정하여 키 입력이 제대로 전달되는지 확인
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    
    UE_LOG(LogTemp, Log, TEXT("키 매핑 설정 시작"));
    UFruitInputMappingManager::ConfigureKeyMappings();
    UE_LOG(LogTemp, Log, TEXT("키 매핑 설정 완료"));

    // 접시 액터를 검색하여 회전 기준 위치로 사용 (접시가 있을 경우)
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if(PlateActors.Num() > 0)
    {
        PlateLocation = PlateActors[0]->GetActorLocation();
    }
    else
    {
        PlateLocation = FVector::ZeroVector;
        UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다. 기본 위치 (0,0,0) 사용."));
    }

    // 초기 카메라(=Pawn) 위치 업데이트: 접시 기준, 30도 내려다봄
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
}

void AFruitPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        InputComponent->BindAction("IncreaseAngle", IE_Pressed, this, &AFruitPlayerController::IncreaseAngle);
        InputComponent->BindAction("DecreaseAngle", IE_Pressed, this, &AFruitPlayerController::DecreaseAngle);
        InputComponent->BindAction("ThrowFruit", IE_Pressed, this, &AFruitPlayerController::ThrowFruit);
        InputComponent->BindAction("RotateCameraLeft", IE_Pressed, this, &AFruitPlayerController::RotateCameraLeft);
        InputComponent->BindAction("RotateCameraRight", IE_Pressed, this, &AFruitPlayerController::RotateCameraRight);
        
        UE_LOG(LogTemp, Log, TEXT("입력 바인딩 완료"));
    }
}

void AFruitPlayerController::IncreaseAngle()
{
    ThrowAngle += AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 증가: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::DecreaseAngle()
{
    ThrowAngle -= AngleStep;
    UE_LOG(LogTemp, Log, TEXT("각도 감소: 현재 각도 %f"), ThrowAngle);
}

void AFruitPlayerController::ThrowFruit()
{
    UE_LOG(LogTemp, Log, TEXT("과일 발사! 각도: %f, 힘: %f"), ThrowAngle, ThrowForce);
    HandleThrow();
}

void AFruitPlayerController::HandleThrow()
{
    // 접시 액터가 있는 위치를 찾음 (클래스 멤버와 이름 충돌 방지를 위해 'LocalSpawnLocation' 사용)
    FVector LocalSpawnLocation = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        LocalSpawnLocation = PlateActors[0]->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("접시 액터 발견: 위치 (%f, %f, %f)"), LocalSpawnLocation.X, LocalSpawnLocation.Y, LocalSpawnLocation.Z);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 액터(Plate)를 찾을 수 없습니다. 기본 위치 (0,0,0)에서 스폰합니다."));
    }

    // FruitBallClass가 설정되어 있으면 과일 액터 스폰
    if (FruitBallClass)
    {
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParams;
        AActor* SpawnedBall = GetWorld()->SpawnActor<AActor>(FruitBallClass, LocalSpawnLocation, SpawnRotation, SpawnParams);
        if (SpawnedBall)
        {
            int32 BallType = FMath::RandRange(1, 11);
            float BaseSize = 25.f;
            float ScaleFactor = 1.f + 0.1f * (BallType - 1);
            float BallSize = BaseSize * ScaleFactor;

            SpawnedBall->SetActorScale3D(FVector(BallSize, BallSize, BallSize));

            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                float RadAngle = FMath::DegreesToRadians(ThrowAngle);
                FVector ImpulseDirection = FVector(FMath::Cos(RadAngle), 0.f, FMath::Sin(RadAngle)).GetSafeNormal();
                PrimComp->AddImpulse(ImpulseDirection * ThrowForce, NAME_None, true);
            }
            UE_LOG(LogTemp, Log, TEXT("공 타입 %d 스폰됨, 크기: %f"), BallType, BallSize);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("공 액터 생성에 실패했습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FruitBallClass가 설정되어 있지 않습니다."));
    }
}

void AFruitPlayerController::RotateCameraLeft()
{
    float DeltaAngle = 10.f;
    CameraOrbitAngle += DeltaAngle;
    if(CameraOrbitAngle >= 360.f)
    {
        CameraOrbitAngle -= 360.f;
    }
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
    UE_LOG(LogTemp, Log, TEXT("카메라 왼쪽으로 회전, 현재 각도: %f"), CameraOrbitAngle);
}

void AFruitPlayerController::RotateCameraRight()
{
    float DeltaAngle = 10.f;
    CameraOrbitAngle -= DeltaAngle;
    if(CameraOrbitAngle < 0.f)
    {
        CameraOrbitAngle += 360.f;
    }
    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(GetPawn(), PlateLocation, CameraOrbitAngle, CameraOrbitRadius, 30.f);
    UE_LOG(LogTemp, Log, TEXT("카메라 오른쪽으로 회전, 현재 각도: %f"), CameraOrbitAngle);
}