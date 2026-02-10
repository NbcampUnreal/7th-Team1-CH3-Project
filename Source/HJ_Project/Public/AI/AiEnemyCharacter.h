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
	void OnEnemyoverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	//이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float MoveSpeed;


	//플레이어와 닿았을때 피해의 정도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	float AttackDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "EnemyStat")
	class AActor* TargetActor;
};
