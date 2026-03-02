#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Gate.generated.h"

class AAiEnemyCharacter;

UCLASS()
class HJ_PROJECT_API AGate : public AActor
{
	GENERATED_BODY()

public:
	AGate();

protected:
	virtual void BeginPlay() override;

	// 데미지 처리 
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

public:
	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	UWidgetComponent* HPWidgetComp;

	// HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Stats")
	float MaxHealth = 500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gate|Stats")
	float Health = 500.0f;

	//관문공격할수있는 좀비수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Combat")
	int32 MaxAttackers = 10;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gate|Combat")
	int32 CurrentAttackers = 0;

	UFUNCTION(BlueprintCallable, Category = "Gate|Combat")
	bool TryRegisterAttacker(AAiEnemyCharacter* Zombie);

	UFUNCTION(BlueprintCallable, Category = "Gate|Combat")
	void UnregisterAttacker(AAiEnemyCharacter* Zombie);

private:
	// ✅ 중복 등록 방지용
	UPROPERTY()
	TSet<TObjectPtr<AAiEnemyCharacter>> RegisteredAttackers;
};