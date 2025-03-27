
#include "UE_FruitMountainGameMode.h"
#include "Manager/FruitPlayerController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"

AUE_FruitMountainGameMode::AUE_FruitMountainGameMode()
{
    // 디폴트 폰 클래스를 지정합니다.
    // 예시: DefaultPawnClass = AYourCharacter::StaticClass();
}

void AUE_FruitMountainGameMode::StartPlay()
{
    Super::StartPlay();
    // 게임 상태 초기화 등 필요한 설정을 여기에 구현합니다.
}

void AUE_FruitMountainGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 매 프레임마다 업데이트되어야 하는 게임 로직을 여기에 구현합니다.
}