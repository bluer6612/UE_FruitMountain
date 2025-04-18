#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeHelper.generated.h"

class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 과일 병합 시도 (새로 추가)
    static void TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint);
    
    // 과일 병합 수행
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);
    
    // 점수 추가
    static void AddScore(int32 BallType);
    
    // 병합 이펙트 재생
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    
    // 병합 시 과일들을 안정화하는 함수
    static void StabilizeFruits(UWorld* World);
    
    // 병합 효과 리소스 미리 로드
    static void PreloadAssets(UWorld* World);

    // 연쇄병합 관련 변수
    static int32 ComboCount;
    static float LastMergeTime;
    static float ComboTimeLimit;
    
    // 연쇄 초기화 함수
    static void ResetCombo();
};