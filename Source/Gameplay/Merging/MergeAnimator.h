#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MergeAnimator.generated.h"

class AFruitBall;
class UStaticMeshComponent;

/**
 * 과일 병합 애니메이션을 담당하는 유틸리티 클래스
 * 병합 시 과일들의 스케일 애니메이션 처리
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UMergeAnimator : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * 병합되는 과일 축소 애니메이션 시작
     * @param Fruit1 - 첫 번째 병합될 과일
     * @param Fruit2 - 두 번째 병합될 과일
     * @param MergeLocation - 병합 위치
     * @param NextBallType - 새로 생성될 과일의 타입
     * @param AnimDuration - 애니메이션 지속 시간 (기본 0.15초)
     * @return 애니메이션 완료 후 콜백 처리를 위한 타이머 핸들
     */
    UFUNCTION(BlueprintCallable, Category = "Fruit Animation")
    static FTimerHandle AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType, float AnimDuration = 0.15f);

    /**
     * 새로 생성된 과일의 성장 애니메이션 시작
     * @param NewFruit - 애니메이션을 적용할 새 과일
     * @param AnimDuration - 애니메이션 지속 시간 (기본 0.15초)
     */
    UFUNCTION(BlueprintCallable, Category = "Fruit Animation")
    static void AnimateNewFruitGrowth(AFruitBall* NewFruit, float AnimDuration = 0.15f);

private:
    /**
     * 병합 애니메이션 틱 처리 함수
     */
    static void TickMergeAnimation(AFruitBall* Fruit1, AFruitBall* Fruit2, float AnimProgress, const FVector& MergeLocation);

    /**
     * 성장 애니메이션 틱 처리 함수
     */
    static void TickGrowthAnimation(AFruitBall* Fruit, float AnimProgress);

    /**
     * 애니메이션 진행도에 따른 스케일 계산
     * 이징(easing) 함수 적용하여 자연스러운 애니메이션 구현
     */
    static float CalculateAnimationScale(float Progress, bool IsGrowing);
};