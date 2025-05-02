#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeHelper.generated.h"

// 전방 선언
class AFruitBall;

// UFruitMergeHelper: 병합 관련 유틸리티 함수들을 제공하는 정적 함수 라이브러리
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 충돌 이벤트 관리
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void RegisterCollisionHandlers(AFruitBall* Fruit);
    
    // 모든 메시 사전 로드 (기존 함수)
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void PreloadAllFruitMeshes(UWorld* World);
    
    // 병합 이펙트
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);

    // 접시에 미리보기 과일 생성
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void SpawnPreviewFruitsOnPlate(UWorld* World);
};