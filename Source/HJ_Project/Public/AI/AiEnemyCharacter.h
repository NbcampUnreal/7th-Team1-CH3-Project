#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "AiEnemyCharacter.generated.h"

class USphereComponent;
class AHJ_Player;

UCLASS()
class HJ_PROJECT_API AAiEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAiEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// =========================
	// HP / Death
	// =========================
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|State")
	bool bIsDead = false;

	// 언리얼 데미지 시스템 진입점(총에서 ApplyPointDamage 하면 여기로 들어옴)
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

protected:
	// 사망 처리(모션 없이 충돌/이동 끄고 일정 시간 뒤 Destroy)
	void Die();

	// =========================
	// Move / Attack
	// =========================
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float MoveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stat")
	float AttackDamage = 5.0f;

	// 공격 틱(몇 초마다 데미지 줄지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float AttackInterval = 1.0f;

	// 공격 범위 스피어(이 범위에 플레이어가 들어오면 일정시간마다 데미지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	USphereComponent* AttackSphere;

	// 현재 공격 중인 플레이어(범위 안에 들어온 플레이어)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	AHJ_Player* OverlappingPlayer = nullptr;

	// 타겟(추적 대상)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
	AActor* TargetActor = nullptr;

	// =========================
	// Stun
	// =========================
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Stun")
	float StunDuration = 1.0f;

protected:
	// 공격 범위 들어옴/나감
	UFUNCTION()
	void OnAttackSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnAttackSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// 타이머로 호출되는 실제 데미지 적용
	void ApplyDamageTick();

	// 경직 해제
	void ResetStun();

	FTimerHandle DamageTimerHandle;
	FTimerHandle StunTimerHandle;
};