#include "PlayerPawn.h"
#include "Throw/FruitPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneComponent.h"

APlayerPawn::APlayerPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // 메시 대신 빈 SceneComponent를 루트로 사용
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // CameraComponent 생성 및 루트에 부착
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(RootComponent);
    CameraComponent->FieldOfView = 90.f;

    FVector CurrentLocation = CameraComponent->GetRelativeLocation();
    CurrentLocation.Y += 10.f;
    CurrentLocation.Z += 125.f;
    CameraComponent->SetRelativeLocation(CurrentLocation);

    FRotator CurrentRotation = CameraComponent->GetRelativeRotation();
    CurrentRotation.Pitch -= 25.f;
    
    CameraComponent->SetRelativeRotation(CurrentRotation);
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}