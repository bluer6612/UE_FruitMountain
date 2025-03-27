#include "UE_FruitMountain.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, FUEFruitMountain, "UE_FruitMountain");

void FUEFruitMountain::StartupModule()
{
    // 게임 초기화 로직을 여기에 추가합니다.
}

void FUEFruitMountain::ShutdownModule()
{
    // 게임 종료 로직을 여기에 추가합니다.
}

void SomeFunction()
{
    // 필요없는 호출은 주석 처리하거나 제거합니다.
    // FUEFruitMountain::Initialize();  
}