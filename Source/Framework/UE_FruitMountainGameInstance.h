#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UE_FruitMountainGameInstance.generated.h"

UENUM(BlueprintType)
enum class EWidgetPosition : uint8
{
    LeftTop,
    LeftMiddle,
    RightTop
};

// 전방 선언
class UUserWidget;
class UFruitUIWidget;

/**
 * 언리얼 엔진용 GameInstance 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UUE_FruitMountainGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // 생성자
    UUE_FruitMountainGameInstance();

    // GameInstance 초기화
    virtual void Init() override;

    // SimpleTextureWidget 표시 함수
    UFUNCTION(BlueprintCallable)
    void CheckPersistentUI();

    UFUNCTION(BlueprintCallable)
    void ShowFruitUIWidget();
};