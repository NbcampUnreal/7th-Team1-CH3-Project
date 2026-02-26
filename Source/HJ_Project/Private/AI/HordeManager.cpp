

#include "AI/HordeManager.h"
#include "AI/AiEnemyCharacter.h"

AHordeManager::AHordeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHordeManager::BeginPlay()
{
	Super::BeginPlay();
}

void AHordeManager::RegisterZombie(AAiEnemyCharacter* Zombie)
{
	if (!IsValid(Zombie)) return;

	AllZombies.Add(Zombie);

	// 첫 좀비는 리더로 지정
	if (!CurrentLeader)
	{
		CurrentLeader = Zombie;
		Zombie->bIsLeader = true;
	}
	else
	{
		Zombie->Leader = CurrentLeader;
		CurrentLeader->Followers.Add(Zombie);
	}
}

void AHordeManager::AssignNewLeader()
{
	for (AAiEnemyCharacter* Z : AllZombies)
	{
		if (IsValid(Z) && !Z->bIsDead)
		{
			CurrentLeader = Z;
			Z->bIsLeader = true;
			return;
		}
	}
}