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

protected:
    virtual void BeginPlay() override;

public:	
    virtual void Tick(float DeltaTime) override;

    // 바닥 Mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Plate")
    class UStaticMeshComponent* BottomMesh;

    // 왼쪽 벽
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Plate")
    class UStaticMeshComponent* LeftWallMesh;

    // 오른쪽 벽
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Plate")
    class UStaticMeshComponent* RightWallMesh;
};