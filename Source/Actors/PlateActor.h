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
    //FVector PlateLocation = FVector(0.f, -10.f, 30.f);
    FVector PlateLocation = FVector(0.f, 0.f, 30.f);
    
    // 통합된 함수 (정적 함수로 선언)
    UFUNCTION(BlueprintCallable, Category = "Plate")
    static FVector CalculatePlateEdge(UWorld* World, float CameraAngle, APlateActor* PlateInstance = nullptr);
    
    // 접시 반경 getter
    UFUNCTION(BlueprintCallable, Category = "Plate")
    float GetPlateRadius() const { return PlateRadius; }
protected:
    virtual void BeginPlay() override;
    
    // 접시 반경 - FruitBall에서 사용됨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Plate")
    float PlateRadius;
    
    // 접시 반경 계산 함수 (BeginPlay에서 호출)
    void CalculatePlateRadius();
    
    // 접시 메시 컴포넌트 - 실제 접시 형태의 메시
    UPROPERTY()
    UStaticMeshComponent* PlateMeshComponent;
    
private:
    // 마지막으로 계산된 스폰 위치 (테스트/최적화용)
    static FVector LastSpawnPos;
    static float LastAngle;
};