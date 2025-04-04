#include "FruitHUD.h"
#include "Engine/Canvas.h"

AFruitHUD::AFruitHUD()
{
    // HUD에서는 별도 초기화 없이 비워둠
}

void AFruitHUD::DrawHUD()
{
    // 부모 클래스 호출
    Super::DrawHUD();

    // 기존에 있던 빨간 박스 · 텍스트 그리기 코드를 전부 삭제:
    // 더 이상 아무것도 그리지 않음
}