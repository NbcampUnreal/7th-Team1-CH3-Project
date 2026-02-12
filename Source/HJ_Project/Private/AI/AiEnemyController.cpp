// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AiEnemyController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h" //AActor와 관련된 모든 기능을 사용 가능하게 해주는 헤더

AAiEnemyController::AAiEnemyController()
{

	bSetControlRotationFromPawnOrientation = true;

	chaseInterval = 0.5f;
}

void AAiEnemyController::BeginPlay()
{
	Super::BeginPlay();
	//좀비 상황판단하는 시간
	GetWorldTimerManager().SetTimer(ChaseTimerHandle,
		this,
		&AAiEnemyController::UpdateChase,
		chaseInterval,
		true);
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
	}

}

void AAiEnemyController::UpdateChase()
{
	//예외처리 좀비,플레이어가 존재하지않을시에 함수 종료
	if (!TargetPlayer || !GetPawn()) return;

	//거리 계산
	float DistanceToPlayer = GetPawn()->GetDistanceTo(TargetPlayer);

	//거리, 상태 판단로직
	if (CurrentState != EAIState::Stunned)//좀비상태가 경직이 아니면
	{
		//플레이어와의 거리가 공격사거리보다 작거나 같을때
		if (DistanceToPlayer <= AttackRange)
		{
			CurrentState = EAIState::AttackingPlayer; //공격해라
		}

		else if (DistanceToPlayer <= DetectionRange)//공격범위는 아닌데 인식이 가능한가?
		{
			CurrentState = EAIState::ChasingPlayer;//인식했으면 쫒아가
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
		if (FinalGate)
		{
			MoveToActor(FinalGate, 100.0f); //관문에 바짝붙으면 안좋을거같아서 거리를 줌
		}
		break;
	case EAIState::ChasingPlayer://플레이어를 인식가능한 범위에 들어왔을떄
		MoveToActor(TargetPlayer, 50.0f);//플레이어와 50.0만큼 주어서 바짝 쫓아가게
		break;
		
	case EAIState::AttackingPlayer://플레이어 공격중
	case EAIState::Stunned://플레이어가 쏜총에 맞아 스턴중
		//공격,스턴중일떄 움직임 정지
		StopMovement();
		break;
	}
}