#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "AiEnemyCharacter.generated.h"

UCLASS()
class HJ_PROJECT_API AAiEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAiEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	//틱마다 플레이어 추격
	virtual void Tick(float DeltaTime) override;

	//플레이어와 닿았을때 피해입기
	UFUNCTION()
	void OnEnemyoverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	//오버랩 끝났을때
	UFUNCTION()
	void OnEnemyEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	//지속 데미지
	void ApplyDamage();

	//ai가 데메지 받았을때 실행
	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	//공격 범위 콜리전
	UPROPERTY(VisibleAnywhere)
	USphereComponent* AttackSphere;

	//겹친 플레이어
	AActor* OverlappingPlayer;

	//지속데미지 타이머
	FTimerHandle DamageTimerHandle;


public:
	//체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float Health = 100.0;


	//이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float MoveSpeed;


	//플레이어와 닿았을때 피해의 정도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float AttackDamage;

	//공격하는 시간 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float AttackInterval = 1.0f;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	class AActor* TargetActor;

	//경직시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Setting")
	float StunDuration = 1.0f;

	//경직초기화
	void ResetStun();

	//경직 계산용 타이머 핸들
	FTimerHandle StunTimerHandle;
};
