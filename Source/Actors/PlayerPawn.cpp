#include "PlayerPawn.h"
#include "Gameplay/Controller/FruitPlayerController.h"
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
    CurrentLocation.Y += 5.f;
    CurrentLocation.Z += 50.f; // 접시 높이의 10배
    CameraComponent->SetRelativeLocation(CurrentLocation);

    FRotator CurrentRotation = CameraComponent->GetRelativeRotation();
    CurrentRotation.Pitch -= 20.f;
    
    CameraComponent->SetRelativeRotation(CurrentRotation);
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}