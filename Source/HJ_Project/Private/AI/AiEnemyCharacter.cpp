
#include "AI/AiEnemyCharacter.h"
#include "Ai/AiEnemyController.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HJ_Player.h"//02/20----10시48분 추가
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

AAiEnemyCharacter::AAiEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// ===== 기본 스탯 =====
	MaxHealth = 100.0f;
	Health = MaxHealth;
	MoveSpeed = 250.0f;
	AttackDamage = 5.0f;
	AttackInterval = 1.0f;
	StunDuration = 1.0f;
	bIsDead = false;

	//무브먼트 최적화
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bUseRVOAvoidance = false;           // 서로 피하는 계산 OFF
		GetCharacterMovement()->bEnablePhysicsInteraction = false; // 캐릭터끼리 밀치기 OFF
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; //설정한 이동 속도 적용
	}

	//캡슐 설정 (좀비끼리는 유령처럼 통과)
	UCapsuleComponent* CapsulePtr = GetCapsuleComponent();
	if (CapsulePtr)
	{
		CapsulePtr->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //물리적 밀치기 안함 충돌만 감지
		CapsulePtr->SetCollisionResponseToAllChannels(ECR_Ignore); //모든 충돌 채널 무시로 초기화 

		CapsulePtr->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 바닥만 밟음
		CapsulePtr->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);    // 좀비끼리는 겹침
		CapsulePtr->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 총알은 통과(메시가 맞음)

		CapsulePtr->SetCanEverAffectNavigation(false); // 내비게이션 렉 방지 (움직일때 마다 길찾기 새로 안하도록)
		CapsulePtr->SetGenerateOverlapEvents(false); //겹치는 이벤트 계산 무시
	}

	//메쉬 설정 (플레이어 길막 + 총알 타격)
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //메쉬 물리연산 X 충돌만 감지
		GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);// 모든 채널 무시

		GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);      // 플레이어는 확실히 길막
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 총알 타격 허용

		// 화면 밖 좀비 애니메이션 연산 안함 (성능 이득)
		GetMesh()->SetCanEverAffectNavigation(false); //메쉬 길찾기에 영향안주게 설정
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered; //카메라에 안잡힌 좀비는 애니메이션 스킵해서 최적화
	}


	// ===== 공격 범위 스피어 =====
	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere")); // 공격 감지용 투명 구체
	AttackSphere->SetupAttachment(RootComponent); // 캐릭터 캡슐에 부착
	AttackSphere->SetSphereRadius(150.f); // 150cm 내에 있으면 공격 

	// 공격 범위는 Overlap만 필요
	AttackSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //물리연산X 겹침만 
	AttackSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널 무시
	AttackSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); //플레이어가 들어왔을때만 오버랩 이벤트 발생
	AttackSphere->SetGenerateOverlapEvents(true); // 오버랩 함수 실행 설정

	// ===== 캡슐(몸통) 충돌 =====
	// 여기서는 기본 캐릭터 설정을 크게 건드리지 않는 게 안전함.
	// (총 트레이스가 안 맞으면 캡슐/메시에서 Visibility=Block만 추가로 맞춰주면 됨)

	
}

void AAiEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 데미지를 받을 수 있게(혹시라도 꺼져있으면 총 데미지가 안 들어옴)
	SetCanBeDamaged(true);

	// 이동 속도
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}

	// 공격 스피어 오버랩 바인딩
	if (AttackSphere)
	{
		AttackSphere->OnComponentBeginOverlap.AddDynamic(
			this, &AAiEnemyCharacter::OnAttackSphereBeginOverlap);

		AttackSphere->OnComponentEndOverlap.AddDynamic(
			this, &AAiEnemyCharacter::OnAttackSphereEndOverlap);
	}

	// 타겟: 플레이어
	TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// 시작 HP 보정
	Health = MaxHealth;
}

void AAiEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAiEnemyCharacter::OnAttackSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bIsDead) return;
	if (!OtherActor || OtherActor == this) return;

	// 태그 방식보다 캐스트가 실수 적음
	AHJ_Player* Player = Cast<AHJ_Player>(OtherActor);
	if (!Player) return;

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] 공격 시작!"));

	OverlappingPlayer = Player;

	// 이미 돌고 있으면 중복 실행 방지
	if (!GetWorldTimerManager().IsTimerActive(DamageTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AAiEnemyCharacter::ApplyDamageTick,
			AttackInterval,
			true
		);
	}
}

void AAiEnemyCharacter::OnAttackSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (bIsDead) return;
	if (!OtherActor) return;

	if (OtherActor == OverlappingPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Zombie] Attack End"));

		OverlappingPlayer = nullptr;
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void AAiEnemyCharacter::ApplyDamageTick()
{
	if (bIsDead) return;
	if (!OverlappingPlayer) return;

	UGameplayStatics::ApplyDamage(
		OverlappingPlayer,
		AttackDamage,
		GetController(),
		this,
		nullptr
	);

	UE_LOG(LogTemp, Warning, TEXT("[Zombie] Deal Damage: %.1f"), AttackDamage);
}

float AAiEnemyCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	//부위별 데메지 배율 계산
	float NewDamageAmount = DamageAmount;//기본 데미지

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		if (PointDamageEvent->HitInfo.PhysMaterial.IsValid())
		{
			FString MaterialName = PointDamageEvent->HitInfo.PhysMaterial->GetName();

			//즉사
			if (MaterialName.Contains(TEXT("PM_Head")))
			{
				NewDamageAmount = Health;
				UE_LOG(LogTemp, Warning, TEXT("[Headshot] Instant Death Applied!"));
			}
		}
	}

	const float ActualDamage = Super::TakeDamage(
		NewDamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	if (ActualDamage <= 0.0f) return 0.0f;

	// 피 감소
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("[Zombie] HP: %f"), Health);

	// 데미지 맞으면 경직 처리 + AI 정지
	AAiEnemyController* AiCon = Cast<AAiEnemyController>(GetController());
	if (AiCon && Health > 0.0f)
	{
		AiCon->CurrentState = EAIState::Stunned;
		AiCon->StopMovement();

		// (선택) 경직 중에는 공격 틱을 잠깐 멈추게(너무 사기면)
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);

		GetWorldTimerManager().ClearTimer(StunTimerHandle);
		GetWorldTimerManager().SetTimer(
			StunTimerHandle,
			this,
			&AAiEnemyCharacter::ResetStun,
			StunDuration,
			false
		);
	}

	// 사망
	if (Health <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AAiEnemyCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	// 타이머 정리(공격/경직)
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	GetWorldTimerManager().ClearTimer(StunTimerHandle);

	// AI 정지
	if (AAiEnemyController* AiCon = Cast<AAiEnemyController>(GetController()))
	{
		AiCon->StopMovement();
		AiCon->CurrentState = EAIState::Stunned;
	}

	// 이동/충돌 끄기
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 1초 후 삭제(바로 없애고 싶으면 Destroy()로 바꿔도 됨)
	SetLifeSpan(1.0f);
}

void AAiEnemyCharacter::ResetStun()
{
	if (bIsDead) return;

	AAiEnemyController* AICon = Cast<AAiEnemyController>(GetController());
	if (AICon)
	{
		AICon->CurrentState = EAIState::MovingToGate;
	}

	// 경직이 풀렸고 아직 플레이어가 범위 안에 있으면 공격 재개
	if (OverlappingPlayer)
	{
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
		GetWorldTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AAiEnemyCharacter::ApplyDamageTick,
			AttackInterval,
			true
		);
	}
}