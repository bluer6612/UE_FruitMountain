#include "GameMode.h"
#include "UE_FruitMountain.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"

AGameMode::AGameMode()
{
    // Set default pawn class to our character
    DefaultPawnClass = AYourCharacter::StaticClass();
}

void AGameMode::StartPlay()
{
    Super::StartPlay();

    // Initialize game state and any other necessary setup
}

void AGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Implement game logic that needs to be updated every frame
}