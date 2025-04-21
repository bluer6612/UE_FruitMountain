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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    FVector PlateScale = FVector(12.f, 12.f, 5.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Plate")
    FVector PlateLocation = FVector(0.f, 0.f, 30.f);
    
    // 접시 반경 획득
    UFUNCTION(BlueprintCallable, Category = "Plate")
    float GetPlateRadius() const { return PlateRadius; }
    
protected:
    virtual void BeginPlay() override;
    
    // 접시 반경 - FruitBall에서 사용됨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
    float PlateRadius = 75.0f;
    
    // 접시 반경 계산 함수 (BeginPlay에서 호출)
    void CalculatePlateRadius();

    // 헤더 파일에 멤버 변수 선언
    UPROPERTY()
    UStaticMeshComponent* PlateMeshComponent;
};