#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FruitBall.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitBall : public AActor
{
    GENERATED_BODY()
    
public:    
    AFruitBall();
    
    virtual void BeginPlay() override;
    
    // 공의 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UStaticMeshComponent* MeshComponent;
    
    // 기본 공 크기 (월드 스케일)
    static constexpr float BaseBallSize = 15.0f;
    
    // 밀도 계수 - 다른 클래스에서 참조할 수 있도록 상수로 정의
    static constexpr float DensityFactor = 100.f;

    // 생성 가능한 공의 최대 레벨
    static constexpr int MaxBallType = 5;
    
    // 공 크기 getter 함수 - 외부에서 공 크기 접근용
    UFUNCTION(BlueprintCallable, Category="Ball Properties")
    float GetBaseBallSize() const { return BaseBallSize; }
    
    // 공 크기 계산 (정적 함수로 구현)
    UFUNCTION(BlueprintCallable, Category="Fruit")
    static float CalculateBallSize(int32 BallType);
    
    // 공 질량 계산 (정적 함수로 구현)
    UFUNCTION(BlueprintCallable, Category="Fruit")
    static float CalculateBallMass(int32 BallType);

    // 충돌 이벤트 핸들러
    UFUNCTION()
    void OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    
    // 접근자 및 설정자 추가
    UFUNCTION()
    int32 GetBallType() const { return BallType; }

    UFUNCTION()
    void SetMerging(bool bMerging) { bIsBeingMerged = bMerging; }

    UFUNCTION()
    bool IsMerging() const { return bIsBeingMerged; }

    UFUNCTION()
    UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

    // 디버그 정보 표시 함수
    UFUNCTION(BlueprintCallable, Category="Debug")
    void DisplayDebugInfo();

    // 미리보기 공 여부 플래그
    UPROPERTY()
    bool bIsPreviewBall = false;
    
    // 접근자/설정자 추가
    UFUNCTION()
    void SetIsPreviewBall(bool bPreview) { bIsPreviewBall = bPreview; }
    
    UFUNCTION()
    bool IsPreviewBall() const { return bIsPreviewBall; }

    // 병합 중인지 여부
    UPROPERTY()
    bool bIsBeingMerged;
    
    // 현재 과일 레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fruit")
    int32 BallType;
};