#include "AI/AiEnemyController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AiEnemyCharacter.h"               
#include "Gate.h"                         
#include "Kismet/GameplayStatics.h"            
#include "Navigation/PathFollowingComponent.h" 
#include "TimerManager.h"

AAiEnemyController::AAiEnemyController()
{
	bSetControlRotationFromPawnOrientation = true;
	// 컨트롤러 회전과 캐릭터 방향 동기화
}

void AAiEnemyController::BeginPlay()
{
	Super::BeginPlay();

	//각 좀비의 UpdateAI 시작 시간을 랜덤하게 만들어서
	//한 프레임에 AI 판단이 몰리는 것을 방지 
	const float RandomDelay = FMath::FRandRange(0.f, UpdateInterval);

	GetWorldTimerManager().SetTimer(
		UpdateTimerHandle,
		this,
		&AAiEnemyController::UpdateAI,
		UpdateInterval,
		true,
		RandomDelay
	);
}

void AAiEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	MyZombie = Cast<AAiEnemyCharacter>(InPawn);

	FindTargets();

	// 스폰되자마자 Gate로 이동
	if (IsValid(TargetGate))
	{
		CurrentState = EAIState::MovingToGate;
		MoveToActor(TargetGate, 50.f);
	}

	// 좀비끼리 겹침 완화
	if (MyZombie && MyZombie->GetCharacterMovement())
	{
		MyZombie->GetCharacterMovement()->bUseRVOAvoidance = true;
	}
}

void AAiEnemyController::FindTargets()
{
	if (GetWorld())
	{
		TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	TArray<AActor*> FoundGates;
	if (GetWorld())
	{
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Gate"), FoundGates);
		if (FoundGates.Num() > 0)
		{
			TargetGate = Cast<AGate>(FoundGates[0]);
		}
	}
}

void AAiEnemyController::UpdateAI()
{
	// 예외처리
	if (!MyZombie || MyZombie->bIsDead) return;

	// 스턴 처리
	if (MyZombie->bIsStunned)
	{
		CurrentState = EAIState::Stunned;
		StopMovement();
		AttackAccTime = 0.f;
		return;
	}

	
	// 팔로워는 리더만 따라감 (최적화)
	if (!MyZombie->bIsLeader && IsValid(MyZombie->Leader))
	{
		MoveToActor(MyZombie->Leader, 120.f);
		return;
	}

	
	// 리더 AI 판단

	const bool bPlayerValid = IsValid(TargetPlayer);
	const bool bGateValid = IsValid(TargetGate);

	float DistToPlayer = BIG_NUMBER;
	if (bPlayerValid)
	{
		DistToPlayer = MyZombie->GetDistanceTo(TargetPlayer);
	}

	float DistToGate = BIG_NUMBER;
	if (bGateValid)
	{
		DistToGate = MyZombie->GetDistanceTo(TargetGate);
	}

	EAIState Prev = CurrentState;

	// 상태 결정

	if (bPlayerValid && DistToPlayer <= AttackRange)
	{
		CurrentState = EAIState::AttackingPlayer;
	}
	else if (bPlayerValid && DistToPlayer <= DetectionRange)
	{
		CurrentState = EAIState::ChasingPlayer;
	}
	else if (bGateValid && DistToGate <= GateAttackEnterRange)
	{
		TryEnterGateAttack();
	}
	else
	{
		CurrentState = EAIState::MovingToGate;
	}

	// ------------------------------
	// 상태에 따른 행동
	// ------------------------------

	switch (CurrentState)
	{
	case EAIState::MovingToGate:
		MoveToGateIfNeeded(Prev);
		AttackAccTime = 0.f;
		break;

	case EAIState::ChasingPlayer:

		// Gate 슬롯 잡고 있으면 해제
		if (MyZombie->RegisteredGate)
		{
			MyZombie->RegisteredGate->UnregisterAttacker(MyZombie);
			MyZombie->RegisteredGate = nullptr;
		}

		MoveToPlayerIfNeeded(Prev);
		AttackAccTime = 0.f;
		break;

	case EAIState::AttackingGate:
	case EAIState::AttackingPlayer:
		StopMovement();
		TickAttack();
		break;

	case EAIState::Stunned:
	default:
		StopMovement();
		AttackAccTime = 0.f;
		break;
	}
}

void AAiEnemyController::MoveToGateIfNeeded(EAIState PrevState)
{
	if (!IsValid(TargetGate)) return;

	if (PrevState != EAIState::MovingToGate ||
		GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		MoveToActor(TargetGate, 50.f);
	}
}

void AAiEnemyController::MoveToPlayerIfNeeded(EAIState PrevState)
{
	if (!IsValid(TargetPlayer)) return;

	if (PrevState != EAIState::ChasingPlayer ||
		GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		MoveToActor(TargetPlayer, 80.f);
	}
}

void AAiEnemyController::TryEnterGateAttack()
{
	if (!IsValid(TargetGate))
	{
		CurrentState = EAIState::MovingToGate;
		return;
	}

	// 이미 슬롯 잡고 있으면 유지
	if (MyZombie->RegisteredGate == TargetGate)
	{
		CurrentState = EAIState::AttackingGate;
		return;
	}

	const bool bSuccess = TargetGate->TryRegisterAttacker(MyZombie);

	if (bSuccess)
	{
		CurrentState = EAIState::AttackingGate;
		MyZombie->RegisteredGate = TargetGate;
		AttackAccTime = 0.f;
	}
	else
	{
		// 슬롯 꽉 찼으면 플레이어로 분산 (플레이어 없으면 게이트 이동)
		CurrentState = IsValid(TargetPlayer) ?
			EAIState::ChasingPlayer :
			EAIState::MovingToGate;
	}
}

void AAiEnemyController::TickAttack()
{
	AttackAccTime += UpdateInterval;

	if (AttackAccTime < MyZombie->AttackInterval) return;

	AttackAccTime = 0.f;

	// Gate 공격
	if (CurrentState == EAIState::AttackingGate)
	{
		if (!IsValid(TargetGate)) return;

		// 거리 벗어나면 공격 취소
		const float Dist = MyZombie->GetDistanceTo(TargetGate);
		if (Dist > GateAttackEnterRange + 50.f)
		{
			TargetGate->UnregisterAttacker(MyZombie);
			MyZombie->RegisteredGate = nullptr;
			CurrentState = EAIState::MovingToGate;
			return;
		}

		MyZombie->BP_OnAttack();

		UGameplayStatics::ApplyDamage(
			TargetGate,
			MyZombie->AttackDamage,
			this,
			MyZombie,
			nullptr
		);
		return;
	}

	
	// 플레이어 공격
	if (CurrentState == EAIState::AttackingPlayer)
	{
		if (!IsValid(TargetPlayer)) return;

		MyZombie->BP_OnAttack();

		UGameplayStatics::ApplyDamage(
			TargetPlayer,
			MyZombie->AttackDamage,
			this,
			MyZombie,
			nullptr
		);
		return;
	}
}