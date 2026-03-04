

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

	//현재 살아있는 리더 세기
	int32 AliveLeaderCount = 0;
	for (AAiEnemyCharacter* Z : AllZombies)
	{
		if (IsValid(Z) && !Z->bIsDead && Z->bIsLeader)
		{
			AliveLeaderCount++;
		}
	}

	//새 리더뽑기
	const int32 AliveCount = AllZombies.Num();
	const bool bNeedMoreLeaders = (AliveLeaderCount == 0) ||
		((AliveCount / (FollowersPerLeader + 1)) > AliveLeaderCount);

	const bool bCanAddLeader = (AliveLeaderCount < MaxLeaders);

	if (bNeedMoreLeaders && bCanAddLeader)
	{
		//새로 들어온 좀비를 리더로
		Zombie->bIsLeader = true;
		Zombie->Leader = nullptr;
		return;
	}

	//팔로워가 적은 리더 따라가기
	AAiEnemyCharacter* BestLeader = nullptr;
	int32 BestFollowerCount = INT32_MAX;

	for (AAiEnemyCharacter* Z : AllZombies)
	{
		if (!IsValid(Z) || Z->bIsDead) continue;
		if (!Z->bIsLeader) continue;

		const int32 FollowerCount = Z->Followers.Num();
		if (FollowerCount < BestFollowerCount)
		{
			BestFollowerCount = FollowerCount;
			BestLeader = Z;
		}
	}

	if (!BestLeader)
	{
		// 리더가 없으면 그냥 리더로 승격
		Zombie->bIsLeader = true;
		Zombie->Leader = nullptr;
		return;
	}

	Zombie->bIsLeader = false;
	Zombie->Leader = BestLeader;
	BestLeader->Followers.Add(Zombie);
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