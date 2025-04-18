#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlateActor.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API APlateActor : public AActor
{
    GENERATED_BODY()
    
public:
    APlateActor();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    FVector PlateScale = FVector(12.f, 12.f, 5.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    //FVector PlateLocation = FVector(0.f, -10.f, 30.f);
    FVector PlateLocation = FVector(0.f, 0.f, 30.f);
};