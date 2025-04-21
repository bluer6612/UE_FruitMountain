#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TextShadowManager.generated.h"

/**
 * UMG 텍스트 블록에 그림자 효과를 관리하는 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UTextShadowManager : public UObject
{
    GENERATED_BODY()
    
public:
    UTextShadowManager();
    
    /**
     * 텍스트 블록에 수평 그림자 효과를 추가합니다.
     * @param InTextBlock 그림자를 추가할 텍스트 블록
     * @param ShadowHeight 그림자의 높이
     * @param ShadowOpacity 그림자의 불투명도 (0.0-1.0)
     * @return 생성된 그림자 이미지에 대한 참조
     */
    UImage* AddHorizontalShadow(UTextBlock* InTextBlock, float ShadowHeight = 4.0f, float ShadowOpacity = 0.3f);
    
    /**
     * 모든 그림자를 숨깁니다.
     */
    void HideAllShadows();
    
    /**
     * 특정 텍스트 블록과 관련된 그림자를 표시하거나 숨깁니다.
     * @param InTextBlock 대상 텍스트 블록
     * @param bVisible 표시 여부
     */
    void SetShadowVisibility(UTextBlock* InTextBlock, bool bVisible);
    
    /**
     * 모든 텍스트 블록 그림자를 제거합니다.
     */
    void RemoveAllShadows();
    
private:
    // 텍스트 블록과 그림자 이미지 매핑
    TMap<UTextBlock*, UImage*> TextToShadowMap;
};