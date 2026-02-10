#include "AI/AiEnemyCharacter.h"
#include "Ai/AiEnemyController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

AAiEnemyCharacter::AAiEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	MoveSpeed = 250.0f;
	AttackDamage = 5.0f;

	//AI컨트롤러 자동으로 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}


void AAiEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	//이동속도
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	if (GetCharacterMovement())//예외처리
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}


	//플레이어와 닿았을때 이벤트 발생
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
			this,
			&AAiEnemyCharacter::OnEnemyoverlap
		);
	}

	//시작할 떄 플레이어를 타겟으로 지정
	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}





//틱마다 실시간 추격
void AAiEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//실시간 추격
	if (TargetActor)
	{
		AAIController* EnemyController = Cast<AAIController>(GetController());

		if (EnemyController)
		{
			//플레이어에게 어디까지 다가갈지
			EnemyController->MoveToActor(TargetActor, 50.0f);//50cm미터정도까지만 다가가
		}
	}
}














void AAiEnemyCharacter::OnEnemyoverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (OtherActor == this) return;

	
	
}

