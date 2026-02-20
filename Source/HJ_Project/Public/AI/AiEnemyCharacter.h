#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

		//ai가 데메지 받았을때 실행
		virtual float TakeDamage(float DamageAmount, 
			struct FDamageEvent const& DamageEvent, 
			class AController* EventInstigator, 
			AActor* DamageCauser) override;
	

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
