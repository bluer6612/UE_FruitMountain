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
    UUE_FruitMountainGameInstance();

    // GameInstance 초기화
    virtual void Init() override;

    // ──────────────────────────────
    // 2) 슬레이트 기반 임시 UI (PersistentUIWidget)
    // ──────────────────────────────
    UFUNCTION(BlueprintCallable)
    void ShowPersistentUI();

    UFUNCTION(BlueprintCallable)
    void CheckPersistentUI();

    // ──────────────────────────────
    // 3) FruitUIWidget 표시
    // ──────────────────────────────
    // 간단한 FruitUI 표시 (중앙)
    UFUNCTION(BlueprintCallable)
    void ShowFruitUIWidget();

    // 이름 변경: 위치 지정 버전
    UFUNCTION(BlueprintCallable)
    void ShowFruitUIWidgetAtPosition(EWidgetPosition InPosition);

    UFUNCTION(BlueprintCallable)
    void ShowAllFruitUI();

    // UMG 위젯 참조
    UPROPERTY()
    UUserWidget* PersistentUIWidget;


private:
    // 슬레이트로 직접 만든 UI를 제거하거나 다시 추가할 때 참조
    TSharedPtr<SWidget> LastAddedViewportContent;
    // FruitUIWidget 참조
    UPROPERTY()
    UUserWidget* FruitWidget;
};