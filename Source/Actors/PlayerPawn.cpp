#include "PlayerPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"

APlayerPawn::APlayerPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // MeshComponent 생성 및 루트 컴포넌트로 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 기본 메시 설정 (예: 엔진 기본 Cube 메시)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if(MeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
        MeshComponent->SetRelativeScale3D(FVector(1.0f));
    }

    // CameraComponent 생성 및 MeshComponent에 부착
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
    CameraComponent->FieldOfView = 90.f;
    // 카메라의 기본 위치 및 회전은 PlayerController에서 UpdateCameraOrbit 등을 통해 조정됩니다.
}

void APlayerPawn::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}