#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "AiEnemyCharacter.generated.h"


class AHJ_Player;
class AGate;
class AHordeManager;
class UPhysicalMaterial;

UCLASS()
class HJ_PROJECT_API AAiEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAiEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	
public:

	/*++++++++++++++++++++++++++++++
	                스탯
	++++++++++++++++++++++++++++++++*/
	//최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float MaxHealth = 100.0f;

	//현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Stat")
	float Health = 100.0f;

	//사망여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsDead = false;

	//스턴
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsStunned = false;

	//공격 데미지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackDamage = 5.0f;

	//공격 속도 조절
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Combat")
	float AttackInterval = 1.0f;
	
	//이동 속도 조절
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Move")
	float MoveSpeed = 250.0f;

	//--------------------
	//헤드샷 == 즉사
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Headshot")
	TObjectPtr<UPhysicalMaterial> HeadshotPhysMat = nullptr;

	//-----------------------------
	//스턴

	//스턴시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy|Stun")
	float StunDuration = 0.6f;

	//조건부 스턴 설정가능
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy|Stun")
	bool bStunOnAnyHit = true;

	//일정 데미지 이상이면 스턴
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy|Stun", meta = (EditCondition = "!bStunOnAnyHit"))
	float MinDamageToStun = 10.0f;

	//Horde
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Enemy|Horde")
	bool bIsLeader = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Enemy|Horde")
	TObjectPtr<AAiEnemyCharacter> Leader = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Enemy|Horde")
	TArray<TObjectPtr<AAiEnemyCharacter>> Followers;

	/*UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Enemy|Horde")
	TObjectPtr<AHordeManager> OwnerHorde = nullptr;*/


	// Gate Slot
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Enemy|Gate")
		TObjectPtr<AGate> RegisteredGate = nullptr;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
	void BP_OnHeadshot();
	//헤드샷 발생 시 BP에서 이펙트/사운드/파티클 처리

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
	void BP_OnDamaged(float Damage, AActor* DamageCauser);
	//피격 반응(사운드/애님몽타주) BP에서 처리

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
	void BP_OnStunStart(float Duration);
	//스턴 시작 애니/이펙트

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
	void BP_OnStunEnd();
	//스턴 끝

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
	void BP_OnDeath();
	//사망 애니/이펙트/드롭 등 BP에서 처리

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	void StartStun();

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	void EndStun();

	UFUNCTION(BlueprintCallable, Category = "Enemy|State")
	void Die();

private:
	FTimerHandle StunTimerHandle;

};