#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlateActor.generated.h"

UCLASS()
// 주석: Plate 액터를 코드로 구현하는 클래스
class UE_FRUITMOUNTAIN_API APlateActor : public AActor
{
    GENERATED_BODY()
    
public:
    APlateActor();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    FVector PlateScale = FVector(10.f, 10.f, 5.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    FVector PlateLocation = FVector(0.f, -10.f, 30.f);
};