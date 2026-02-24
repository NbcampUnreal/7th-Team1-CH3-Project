// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AiEnemyController.h"
#include "AI/AiEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Actor.h" //AActor와 관련된 모든 기능을 사용 가능하게 해주는 헤더

AAiEnemyController::AAiEnemyController()
{

	bSetControlRotationFromPawnOrientation = true;

	chaseInterval = 0.5f;

	DamageAccTime = 0.0f;
}

void AAiEnemyController::BeginPlay()
{
	Super::BeginPlay();

	float RandomDelay = FMath::FRandRange(0.0f, chaseInterval); //공격하는 시간을 일치하는게 아니라 각각 엇박자로 계산


	//좀비 상황판단하는 시간
	GetWorldTimerManager().SetTimer(ChaseTimerHandle,
		this,
		&AAiEnemyController::UpdateChase,
		chaseInterval,
		true,
		RandomDelay);//이게 엇박자로 생각
}

void AAiEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);


	if (GetWorld())
	{
		//0번 플레이어 컨트롤러의 캐릭터를 찾음
		TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		//최종관문에 지정해둔 태그 를 찾는 액터 자동으로 찾는거임
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Gate"), FoundGates);
		if (FoundGates.Num() > 0)
		{
			FinalGate = FoundGates[0];
		}

		MyZombieCharacter = Cast<AAiEnemyCharacter>(InPawn);//캐스팅
	}

}

void AAiEnemyController::UpdateChase()
{
	//예외처리 좀비,플레이어가 존재하지않을시에 함수 종료
	if (!GetPawn() || !MyZombieCharacter || MyZombieCharacter->bIsDead) return;
	//관문이 파과 되었는가?
	bool bIsGateValid = IsValid(FinalGate);

	//플레이어 거리 계산
	float DistanceToPlayer = 999999.0f;
	if (TargetPlayer)
	{
		DistanceToPlayer = GetPawn()->GetDistanceTo(TargetPlayer);
	}

	//관문과의 거리 계산
	float DistanceToGate = 999999.0f;
	if (bIsGateValid)
	{
		DistanceToGate = GetPawn()->GetDistanceTo(FinalGate);
	}

	EAIState PreviousState = CurrentState; //상태 변경 감지 (현재상태 백업)

	//거리, 상태 판단로직
	if (CurrentState != EAIState::Stunned)//좀비상태가 경직이 아니면
	{
		//플레이어와의 거리가 공격사거리보다 작거나 같을때
		if (TargetPlayer && DistanceToPlayer <= AttackRange)
		{
			CurrentState = EAIState::AttackingPlayer; //공격해라
		}

		else if (TargetPlayer && DistanceToPlayer <= DetectionRange)//공격범위는 아닌데 인식이 가능한가?
		{
			CurrentState = EAIState::ChasingPlayer;//인식했으면 쫒아가
		}

		else if (bIsGateValid && DistanceToGate <= 350.0f)//관문 공격 상태
		{
			CurrentState = EAIState::AttackingGate;
		}

		else // 위에 조건들이 맞지 않으면 
		{
			CurrentState = EAIState::MovingToGate;//최종관문을 향해서 가라
		}

	}

	//판단 결정하고 실제 행동
	switch (CurrentState)
	{
	case EAIState::MovingToGate://플레이어를 인식못하고 최종관문을 향하는 결정을 했을때
		if (bIsGateValid)
		{
			if (PreviousState != EAIState::MovingToGate || GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				MoveToActor(FinalGate, 10.0f); //관문에 바짝붙으면 안좋을거같아서 거리를 줌
			}
		}
		break;
	case EAIState::ChasingPlayer://플레이어를 인식가능한 범위에 들어왔을떄
		if (TargetPlayer)//플레이어와 50.0만큼 주어서 바짝 쫓아가게
		{
			//공격해야할 대상이 바뀌었을때 민 호출
			if (PreviousState != EAIState::ChasingPlayer || GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				MoveToActor(TargetPlayer, 50.0f);
			}
		}
		break;

	case EAIState::AttackingPlayer://플레이어 공격중
	case EAIState::AttackingGate: // 관문 공격시 이동 정지
	case EAIState::Stunned://플레이어가 쏜총에 맞아 스턴중
		//공격,스턴중일떄 움직임 정지
		StopMovement();
		break;
	}

	//좀비 캐릭터 변수 사용
	if (CurrentState == EAIState::AttackingGate && bIsGateValid)
	{
		DamageAccTime += chaseInterval;// 시간 누적

		//좀비 캐릭터 공격 간격 비교
		if (DamageAccTime >= MyZombieCharacter->AttackInterval)
		{
			UGameplayStatics::ApplyDamage(//캐릭터에 저장된 데미지로 관문에 데미지
				FinalGate,
				MyZombieCharacter->AttackDamage,
				this,
				MyZombieCharacter,
				nullptr
			);

			//데미지 줐으니 시간 누적 초기화
			DamageAccTime = 0.0f;

			// 로그로 확인 (나중에 지워도 됨)
			UE_LOG(LogTemp, Warning, TEXT("관문 공격! 데미지: %f"), MyZombieCharacter->AttackDamage);
		}
	}

	else
	{
		//다시 붙었을때 즉시 공격 방지
		DamageAccTime = 0.0f;
	}
}


