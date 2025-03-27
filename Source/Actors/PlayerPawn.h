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

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
    // 기본 메시 컴포넌트 (Pawn의 시각적 표현)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* MeshComponent;

    // 카메라 컴포넌트 (Pawn에 부착하여 사용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* CameraComponent;
};