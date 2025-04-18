#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API APlayerPawn : public APawn
{
    GENERATED_BODY()

public:
    APlayerPawn();
    
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 카메라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraComponent* CameraComponent;
};