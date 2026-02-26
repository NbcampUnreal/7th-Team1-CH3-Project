

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
	Zombie->OwnerHorde = this;

	// 첫 좀비는 리더로 지정
	if (!CurrentLeader)
	{
		CurrentLeader = Zombie;
		Zombie->bIsLeader = true;
		Zombie->Leader = nullptr;
	}
	else
	{
		Zombie->bIsLeader = false;
		Zombie->Leader = CurrentLeader;
		CurrentLeader->Followers.Add(Zombie);
	}
}


//좀비가 죽을 때 호출
void AHordeManager::NotifyZombieDied(AAiEnemyCharacter* Zombie)
{
	if (!Zombie) return;

	//죽은 좀비 제거
	AllZombies.Remove(Zombie);

	// 리더가 죽었으면 새 리더 선정
	if (Zombie == CurrentLeader)
	{
		CurrentLeader = nullptr;
		AssignNewLeader();
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
			Z->Leader = nullptr;

			// 나머지는 새 리더 따라가게 재설정
			CurrentLeader->Followers.Empty();

			for (AAiEnemyCharacter* Other : AllZombies)
			{
				if (!IsValid(Other) || Other->bIsDead) continue;
				if (Other == CurrentLeader) continue;

				Other->bIsLeader = false;
				Other->Leader = CurrentLeader;
				CurrentLeader->Followers.Add(Other);
			}
			return;
		}
	}
}