// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AiEnemyController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

AAiEnemyController::AAiEnemyController()
{


	bSetControlRotationFromPawnOrientation = true;
}

void AAiEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//0번 플레이어 컨트롤러의 캐릭터를 찾음
	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	//플레이어존재시에 0.5초마다 경로 갱신
	if (TargetPlayer)
	{
		GetWorldTimerManager().SetTimer(ChaseTimerHandle,
			this,
			&AAiEnemyController::UpdateChase,
			0.5f,
			true);
	}

}

void AAiEnemyController::UpdateChase()
{

	if (TargetPlayer && GetPawn())
	{
		MoveToActor(TargetPlayer, 50.0f);
	}


}

