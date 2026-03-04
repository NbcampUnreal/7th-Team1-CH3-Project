#include "AI/AiEnemyCharacter.h"
#include "AI/HordeManager.h"
#include "AI/AiEnemyController.h"
#include "Gate.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HJ_GameMode.h"

AAiEnemyCharacter::AAiEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	//Tick 완전 OFF

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	// 배치/스폰 시 자동으로 AI 컨트롤러 붙음

	
	// 이동 최적화
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bUseRVOAvoidance = false;
		// RVO 끔 (좀비 많을 때 CPU 많이 먹음)

		Move->bEnablePhysicsInteraction = false;
		// 물리 밀림 연산 제거

		Move->MaxWalkSpeed = MoveSpeed;
		// BP에서 설정한 이동 속도 적용
	}

	
	// 충돌 최적화
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// 물리 충돌 대신 쿼리만

		Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		// 좀비끼리 겹쳐도 밀림 연산 안함 (렉 감소)
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
        MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		// 총알 라인트레이스만 받음

        MeshComp->VisibilityBasedAnimTickOption =
			EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		// 화면 밖 애니메이션 연산 제거
	}
}


void AAiEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	// 시작 시 체력 초기화
}


//데미지 받기
float AAiEnemyCharacter::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    if (bIsDead) return 0.f;
    // 이미 죽었으면 무시

    // Super 호출
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    const float AppliedDamage = (ActualDamage > 0.f) ? ActualDamage : DamageAmount;


    //헤드샷 즉사 처리
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* Point =
            static_cast<const FPointDamageEvent*>(&DamageEvent);

        UPhysicalMaterial* HitPM =
            Point->HitInfo.PhysMaterial.Get();

        // PhysMaterial 이름으로 판정 (가장 안정적)
        if (HitPM && HitPM->GetName().Contains(TEXT("PM_Head")))
        {
            BP_OnHeadshot();  //BP에서 연출 처리
            Die();
            return AppliedDamage;
        }
    }


    //일반 데미지 처리
    Health = FMath::Clamp(Health - AppliedDamage, 0.f, MaxHealth);

    BP_OnDamaged(AppliedDamage, DamageCauser);
    // BP 피격 연출
    if (HitSound && !bIsDead)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            HitSound,
            GetActorLocation()
        );
    }


    //스턴 처리
    bool bShouldStun = false;

    if (Health > 0.0f) // 살아있을 때만
    {
        if (bStunOnAnyHit)
        {
            bShouldStun = true;
        }
        else if (AppliedDamage >= MinDamageToStun)
        {
            bShouldStun = true; 
        }
    }

    if (bShouldStun)
    {
        StartStun();
    }


    //사망 체크
    if (Health <= 0.0f)
    {
        Die();
    }

    return AppliedDamage;
}


//스턴
void AAiEnemyCharacter::StartStun()
{
    if (bIsDead) return;

    bIsStunned = true;

    BP_OnStunStart(StunDuration);
    // BP에서 애니,이펙트

    GetWorldTimerManager().ClearTimer(StunTimerHandle);

    GetWorldTimerManager().SetTimer(
        StunTimerHandle,
        this,
        &AAiEnemyCharacter::EndStun,
        StunDuration,
        false
    );

    // AI 이동 정지
    if (AAiEnemyController* AIC =
        Cast<AAiEnemyController>(GetController()))
    {
        AIC->StopMovement();
    }
}


//스턴 종료
void AAiEnemyCharacter::EndStun()
{
    if (bIsDead) return;

    bIsStunned = false;

    BP_OnStunEnd();
}


//사망처리
void AAiEnemyCharacter::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("Die Called"));
    if (bIsDead) return;
    bIsDead = true;

    GetWorldTimerManager().ClearTimer(StunTimerHandle);

    //Gate 슬롯 해제
    if (RegisteredGate)
    {
        RegisteredGate->UnregisterAttacker(this);
        RegisteredGate = nullptr;
    }

    // AI 정지
    if (AAiEnemyController* AIC =
        Cast<AAiEnemyController>(GetController()))
    {
        AIC->StopMovement();
    }

    // 이동/충돌 끄기
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->DisableMovement();
    }

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Horde 리더 사망 처리
    if (OwnerHorde)
    {
        OwnerHorde->NotifyZombieDied(this);
    }

    // GameMode에 웨이브 카운트 알림
    if (AHJ_GameMode* GM =
        Cast<AHJ_GameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->OnZombieKilled();
    }


    BP_OnDeath();
    // BP에서 사망 애니/이펙트

    SetLifeSpan(1.5f);
    // 일정 시간 후 자동 제거
}