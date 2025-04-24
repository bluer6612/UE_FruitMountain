// MergeAnimationManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MergeAnimationManager.generated.h"

class AFruitBall;

// 병합 애니메이션 상태를 관리하는 전용 객체
UCLASS()
class UE_FRUITMOUNTAIN_API UMergeAnimationManager : public UObject
{
    GENERATED_BODY()

public:
    UMergeAnimationManager();
    
    // 애니메이션 시작
    UFUNCTION()
    void StartAnimation(AFruitBall* SourceFruit1, AFruitBall* SourceFruit2, 
                        const FVector& MergeLocation, int32 NextBallType);
    
    // 애니메이션 업데이트 
    UFUNCTION()
    void UpdateAnimation(float DeltaTime);
    
    // 애니메이션 완료 처리
    UFUNCTION()
    void CompleteAnimation();
    
    // 생성된 과일 반환
    UFUNCTION()
    AFruitBall* GetCreatedFruit() const { return NewFruit; }
    
    // 애니메이션 진행 중인지 확인
    UFUNCTION()
    bool IsAnimating() const { return bIsAnimating; }

private:
    // 애니메이션 대상 과일
    UPROPERTY()
    AFruitBall* Fruit1;
    
    UPROPERTY()
    AFruitBall* Fruit2;
    
    UPROPERTY()
    AFruitBall* NewFruit;
    
    // 애니메이션 상태
    bool bIsAnimating;
    bool bIsCompleted;
    
    // 애니메이션 파라미터
    float AnimationTime;
    float TotalDuration;
    FVector MergePoint;
    int32 NextFruitType;
    
    // 초기 스케일 저장
    FVector InitialScale1;
    FVector InitialScale2;
};