#include "AI/AiEnemyCharacter.h"
#include "Ai/AiEnemyController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"


//캐릭터
AAiEnemyCharacter::AAiEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	MoveSpeed = 250.0f;
	AttackDamage = 5.0f;
	Health = 100.0f;
	StunDuration = 1.0f;

	//AI컨트롤러 자동으로 빙의
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}



//비긴플레이
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

}



//오버랩
void AAiEnemyCharacter::OnEnemyoverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	if (OtherActor->ActorHasTag(TEXT("Player")))
	{

		if (GEngine)
		{
			//디버드 메세지 테스트끝나면 지워도됨
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("적: 플레이어와 충돌함!"));
		}

		//삭제 가능
		UE_LOG(LogTemp, Warning, TEXT("Enemy Overlapped with Player!"));

		UGameplayStatics::ApplyDamage(OtherActor,
			AttackDamage,
			GetController(),
			this,
			nullptr);

	}
}



//데메지
float AAiEnemyCharacter::TakeDamage(float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);
//체력감소
	Health -= ActualDamage;

	//ai컨트롤러 
	AAiEnemyController* AiCon = Cast<AAiEnemyController>(GetController());
	if (AiCon && Health > 0)
	{
		//경직상태로 변경
		AiCon->CurrentState = EAIState::Stunned;
		AiCon->StopMovement();

		//경직 갱신(타이머설정)
		GetWorldTimerManager().ClearTimer(StunTimerHandle);
		GetWorldTimerManager().SetTimer(StunTimerHandle,
			this,
			&AAiEnemyCharacter::ResetStun,
			StunDuration,
			false);
	}
	if (Health <= 0)
	{
		//죽음
		Destroy();
	}
	return ActualDamage;
}



//경직 끝날을때
void AAiEnemyCharacter::ResetStun()
{
	AAiEnemyController* AICon = Cast<AAiEnemyController>(GetController());
	if (AICon)
	{
		// 상태를 다시 기본 이동 상태로 변경
		AICon->CurrentState = EAIState::MovingToGate;
	}
}

