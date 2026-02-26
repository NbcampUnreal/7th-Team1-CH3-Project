
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
	
	// 컴포넨트
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	UWidgetComponent* HPWidgetComp;

	
	// HP
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Stats")
	float Health = 500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Gate|Stats")
	float MaxHealth = 500.0f;

	// Attacker Slot System (추가)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gate|Combat")
	int32 MaxAttackers = 10;
	// 동시에 게이트를 때릴 수 있는 좀비 수 (BP에서 쉽게 조절)

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gate|Combat")
	int32 CurrentAttackers = 0;
	// 현재 게이트를 때리는 좀비 수 (디버그용)

	UFUNCTION(BlueprintCallable, Category = "Gate|Combat")
	bool TryRegisterAttacker(AAiEnemyCharacter* Zombie);
	//공격 슬롯 등록 시도 (성공 true / 실패 false)

	UFUNCTION(BlueprintCallable, Category = "Gate|Combat")
	void UnregisterAttacker(AAiEnemyCharacter* Zombie);
	//공격 중이던 좀비가 죽거나 떠나면 슬롯 해제
};