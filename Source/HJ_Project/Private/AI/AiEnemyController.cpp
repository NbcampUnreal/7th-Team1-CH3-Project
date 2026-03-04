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
	LastTimeSeenPlayer = -9999.0f;
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

	//좀비중 일부 게이트 전담
	if (MyZombie && !MyZombie->bIsLeader)
	{
		//25%sms 관문 전담
		MyZombie->bGateRunner = (FMath::FRand() < 0.25f);
	}


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

//스폰위치 계산
FVector AAiEnemyController::GetRingOffsetAround(const FVector& Center, int32 Slot, int32 Slots, float Radius) const
{
	if (Slots <= 0) return FVector::ZeroVector;

	const float T = float(Slot) / float(Slots);
	const float Angle = 2.0f * PI * T;
	//평면에서 원형 배치
	return FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
}

//안정적으로 배치
int32 AAiEnemyController::GetStableSlot(int32 Slots) const
{
	if (Slots <= 0) return 0;
	return int32(GetUniqueID() % Slots);
}

//리더 주변 원형 자리로 MoveToLocation을 걸어 분산시킴.
void AAiEnemyController::MoveAsFollowerFormation()
{
	if (!MyZombie || MyZombie->bIsDead) return;
	if (!IsValid(MyZombie->Leader)) return;

	const FVector LeaderLoc = MyZombie->Leader->GetActorLocation();
	const int32 Slot = GetStableSlot(FollowerRingSlots);
	const FVector Goal = LeaderLoc + GetRingOffsetAround(LeaderLoc, Slot, FollowerRingSlots, FollowerRingRadius);

	// 분산 목표점으로 이동
	MoveToLocation(
		Goal,
		FormationAcceptanceRadius, 
		false,                    
		true,                      
		true,                     
		false,                     
		nullptr,                  
		true                       
	);
}


//플레이어 주변 포위
void AAiEnemyController::MoveToPlayerSurround()
{
	if (!MyZombie || MyZombie->bIsDead) return;
	if (!IsValid(TargetPlayer)) return;

	const FVector PlayerLoc = TargetPlayer->GetActorLocation();

	const int32 Slot = GetStableSlot(PlayerSurroundSlots);
	const FVector Goal = PlayerLoc + GetRingOffsetAround(PlayerLoc, Slot, PlayerSurroundSlots, PlayerSurroundRadius);

	MoveToLocation(
		Goal,
		FormationAcceptanceRadius,
		false,
		true,
		true,
		false,
		nullptr,
		true
	);
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

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	//게이트전담은 무조건 게이트로만
	if (!MyZombie->bIsLeader)
	{
		if (MyZombie->bGateRunner && IsValid(TargetGate))
		{
			EAIState Prev = CurrentState;
			CurrentState = EAIState::MovingToGate;
			MoveToGateIfNeeded(Prev);
			return;
		}
	}

	// 공통 타겟/거리 계산(중복 방지)
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

	//팔로워도 공격/추격 허용
	//가까우면 공격
	if (!MyZombie->bIsLeader && IsValid(MyZombie->Leader))
	{
		//게이트 근처면 팔로워도 관문 공격
		if (bGateValid && DistToGate <= GateAttackEnterRange)
		{
			TryEnterGateAttack();
			if (CurrentState == EAIState::AttackingGate)
			{
				StopMovement();
				TickAttack();
				return;
			}
			// 슬롯이 꽉 차서 실패한 경우는 아래 로직(플레이어/리더)로 흘려도 됨
		}

		// 가까우면 공격
		if (bPlayerValid && DistToPlayer <= AttackRange)
		{
			CurrentState = EAIState::AttackingPlayer;
			StopMovement();
			TickAttack();
			return;
		}

		// 일정 거리 내면 추격(또는 포위)
		if (bPlayerValid && DistToPlayer <= ChaseDropRange)
		{
			EAIState Prev = CurrentState;
			CurrentState = EAIState::ChasingPlayer;
			MoveToPlayerIfNeeded(Prev);
			AttackAccTime = 0.0f;
			return;
		}

		// 아니면 리더 따라가기
		MoveAsFollowerFormation();
		return;
	}

	// 리더 AI 판단
	// 최근에 플레이어를 본 시간 갱신 (딱 1번만)
	if (bPlayerValid && DistToPlayer <= DetectionRange)
	{
		LastTimeSeenPlayer = Now;
	}

	//추격 유지 여부
	const bool bRecentlySeen =
		bPlayerValid && ((Now - LastTimeSeenPlayer) <= LoseSightDelay);

	const bool bWithinChaseDrop =
		bPlayerValid && (DistToPlayer <= ChaseDropRange);

	const bool bTooFarFromGate =
		bGateValid && (DistToGate > MaxChaseFromGate);

	const bool bShouldChase =
		bRecentlySeen && bWithinChaseDrop && !bTooFarFromGate;

	EAIState Prev = CurrentState;

	// 상태 결정
	if (bPlayerValid && DistToPlayer <= AttackRange)
	{
		CurrentState = EAIState::AttackingPlayer;
	}
	else if (bShouldChase)
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

	// 상태에 따른 행동
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
		AttackAccTime = 0.0f;
		break;

	case EAIState::AttackingGate:
		StopMovement();
		TickAttack();
		break;

	case EAIState::AttackingPlayer:
	{
		//공격 상태에서도 거리 벌어지면 다시 포위 위치로 재배치
		if (IsValid(TargetPlayer))
		{
			const float Dist = MyZombie->GetDistanceTo(TargetPlayer);
			if (Dist > AttackRange * 0.9f)
			{
				MoveToPlayerSurround();
				AttackAccTime = 0.f;
				break;
			}
		}

		StopMovement();
		TickAttack();
		break;
	}

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