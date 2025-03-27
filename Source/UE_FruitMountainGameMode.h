#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

// 필요한 추가 include가 있다면 여기에 삽입...

#include "UE_FruitMountainGameMode.generated.h"

UCLASS()
// 주석: UE_FRUITMOUNTAIN_API를 사용하여 모듈 공개
class UE_FRUITMOUNTAIN_API AUE_FruitMountainGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AUE_FruitMountainGameMode();

    // StartPlay와 Tick 함수의 오버라이드 선언 추가
    virtual void StartPlay() override;
    virtual void Tick(float DeltaTime) override;

    // 여기에 클래스 내용을 작성하세요.
};