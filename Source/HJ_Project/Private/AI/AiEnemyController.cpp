// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AiEnemyController.h"

#include "AI/AiEnemyCharacter.h"               // 내 좀비
#include "Gate.h"                         // Gate 슬롯/검사
#include "Kismet/GameplayStatics.h"            // 플레이어 찾기, 태그 찾기
#include "Navigation/PathFollowingComponent.h" // GetMoveStatus
#include "TimerManager.h"                      // 타이머

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
		UpdateTimerHandle,            // 타이머 핸들
		this,                         // 실행 객체
		&AAiEnemyController::UpdateAI,// 호출 함수
		UpdateInterval,               // 주기
		true,                         // 반복
		RandomDelay                   // 시작 딜레이(엇박자)
	);
}

void AAiEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	MyZombie = Cast<AAiEnemyCharacter>(InPawn);
	// 컨트롤러가 소유한 Pawn을 좀비로 캐스팅

	FindTargets();
	// 플레이어/Gate 찾기

	// 스폰되자마자 Gate로 이동
	if (IsValid(TargetGate))
	{
		CurrentState = EAIState::MovingToGate;
		MoveToActor(TargetGate, 50.f);
	}
}

void AAiEnemyController::FindTargets()
{
	// 플레이어 찾기
	if (GetWorld())
	{
		TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	// Gate 찾기
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

	
	//스턴 처리
	if (MyZombie->bIsStunned)
	{
		CurrentState = EAIState::Stunned;
		StopMovement();
		AttackAccTime = 0.f;
		return;
	}

	
	// 2) 팔로워 최적화: 리더만 따라감
	if (!MyZombie->bIsLeader && IsValid(MyZombie->Leader))
	{
		CurrentState = EAIState::MovingToGate;

		// 팔로워는 리더만 따라감
		MoveToActor(MyZombie->Leader, 120.f);
		return;
	}

	
	// 3) 리더 AI 판단 (거리 계산)
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

	
	//상태 결정
	// 플레이어가 공격 범위면 플레이어 공격
	if (bPlayerValid && DistToPlayer <= AttackRange)
	{
		CurrentState = EAIState::AttackingPlayer;
	}
	// 플레이어가 인식 범위면 추격 (리더만)
	else if (bPlayerValid && DistToPlayer <= DetectionRange)
	{
		CurrentState = EAIState::ChasingPlayer;
	}
	// 게이트 근처면 게이트 공격 시도 
	else if (bGateValid && DistToGate <= GateAttackEnterRange)
	{
		// 게이트 공격  시도
		TryEnterGateAttack();
	}
	else
	{
		// 기본은 게이트로 이동
		CurrentState = EAIState::MovingToGate;
	}

	
	// 5) 상태에 따른 행동
	
	switch (CurrentState)
	{
	case EAIState::MovingToGate:
		MoveToGateIfNeeded(Prev);
		AttackAccTime = 0.f;
		break;

	case EAIState::ChasingPlayer:
		MoveToPlayerIfNeeded(Prev);

		// Gate 슬롯 잡고 있으면 해제 
		if (MyZombie->RegisteredGate)
		{
			MyZombie->RegisteredGate->UnregisterAttacker(MyZombie);
			MyZombie->RegisteredGate = nullptr;
		}

		AttackAccTime = 0.f;
		break;

	case EAIState::AttackingGate:
	case EAIState::AttackingPlayer:
		StopMovement();
		TickAttack(); // 공격 누적/실행
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

	// MoveTo를 매번 호출하면 비용이 큼
	if (PrevState != EAIState::MovingToGate || GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		MoveToActor(TargetGate, 50.f);
	}
}

void AAiEnemyController::MoveToPlayerIfNeeded(EAIState PrevState)
{
	if (!IsValid(TargetPlayer)) return;

	if (PrevState != EAIState::ChasingPlayer || GetMoveStatus() == EPathFollowingStatus::Idle)
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

	// 이미 슬롯 잡고 공격 중이면 그대로 유지
	if (MyZombie->RegisteredGate == TargetGate)
	{
		CurrentState = EAIState::AttackingGate;
		return;
	}

	// 슬롯 등록 시도
	const bool bSuccess = TargetGate->TryRegisterAttacker(MyZombie);

	if (bSuccess)
	{
		CurrentState = EAIState::AttackingGate;
		MyZombie->RegisteredGate = TargetGate;
		AttackAccTime = 0.f;
	}
	else
	{
		// 슬롯이 꽉 찼으면 플레이어로 분산
		CurrentState = EAIState::ChasingPlayer;
	}
}

void AAiEnemyController::TickAttack()
{
	// 공격 시간 누적
	AttackAccTime += UpdateInterval;

	// 공격 간격이 되면 1번 공격
	if (AttackAccTime < MyZombie->AttackInterval) return;

	AttackAccTime = 0.f;

	// Gate 공격
	if (CurrentState == EAIState::AttackingGate)
	{
		if (!IsValid(TargetGate)) return;

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