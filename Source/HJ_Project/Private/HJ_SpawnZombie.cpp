#include "HJ_SpawnZombie.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/AiEnemyCharacter.h"
#include "AI/HordeManager.h"
#include "Engine/DataTable.h"

AHJ_SpawnZombie::AHJ_SpawnZombie()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;
}

void AHJ_SpawnZombie::BeginPlay()
{
	Super::BeginPlay();
}

FZombieSpawnRow* AHJ_SpawnZombie::FindWave(int32 WaveNumber)
{
	if (!WaveDataTable) return nullptr;

	FName RowName = FName(*FString::FromInt(WaveNumber));

	return WaveDataTable->FindRow<FZombieSpawnRow>(
		RowName,
		TEXT("WaveLookup")
	);
}

void AHJ_SpawnZombie::SpawnWave(int32 WaveNumber)
{
	if (!GetWorld()) return;

	FZombieSpawnRow* Row = FindWave(WaveNumber);
	if (!Row || !Row->ZombieClass) return;

	for (int32 i = 0; i < Row->SpawnCount; i++)
	{
		FVector SpawnLoc = GetRandomPointInBox();

		AActor* Spawned = GetWorld()->SpawnActor<AActor>(
			Row->ZombieClass,
			SpawnLoc,
			FRotator::ZeroRotator
		);

		if (Spawned && HordeManager)
		{
			if (AAiEnemyCharacter* Zombie = Cast<AAiEnemyCharacter>(Spawned))
			{
				HordeManager->RegisterZombie(Zombie);
			}
		}
	}
}

FVector AHJ_SpawnZombie::GetRandomPointInBox() const
{
	FVector Origin = SpawnBox->GetComponentLocation();
	FVector Extent = SpawnBox->GetScaledBoxExtent();

	return UKismetMathLibrary::RandomPointInBoundingBox(
		Origin,
		Extent
	);
}