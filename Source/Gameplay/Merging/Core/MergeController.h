#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MergeController.generated.h"

class AFruitBall;

// AMergeController: 병합 프로세스 전체를 관리하는 싱글톤 컨트롤러
UCLASS()
class UE_FRUITMOUNTAIN_API AMergeController : public AActor
{
    GENERATED_BODY()
    
public:
    AMergeController();
    
    // 싱글톤 인스턴스 접근
    UFUNCTION(BlueprintCallable, Category="Merge System")
    static AMergeController* Get(const UObject* WorldContext);
    
    // 병합 프로세스 시작
    UFUNCTION(BlueprintCallable, Category="Merge System")
    bool StartMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& CollisionPoint);
    
    // 병합 진행 중 여부 확인
    UFUNCTION(BlueprintCallable, Category="Merge System")
    bool IsMergeInProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Merging")
    void SetMergeInProgress(bool bInProgress);
    
    // 병합 완료 처리 함수
    void CompleteMerge();
    
    // 병합 이펙트 재생 함수
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    

protected:
    virtual void BeginPlay() override;

private:
    // 싱글톤 인스턴스
    static AMergeController* Instance;
    
    // 병합 진행 중 상태 변수 추가
    UPROPERTY()
    bool bMergeInProgress;
};