#include "UE_FruitMountainGameMode.h"
// #include "GameMode.h"         // 제거: 엔진 기본 GameMode 헤더와 충돌
#include "UE_FruitMountain.h"  // 수정 후 예시: 모듈 헤더로 추정되며 불필요함
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

void Function()
{
    // UE_FruitMountain::Initialize();  // 수정: 이 호출은 잘못된 이름을 사용하고 있으므로 제거합니다.
}