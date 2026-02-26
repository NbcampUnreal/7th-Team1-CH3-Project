#include "HJ_SpawnZombie.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

AHJ_SpawnZombie::AHJ_SpawnZombie()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;

	CurrentWave = 1;
}

void AHJ_SpawnZombie::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		StartWave(CurrentWave);
	}
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

void AHJ_SpawnZombie::StartWave(int32 WaveNumber)
{
	if (!GetWorld()) return;

	FZombieSpawnRow* Row = FindWave(WaveNumber);
	if (!Row || !Row->ZombieClass) return;

	for (int32 i = 0; i < Row->SpawnCount; i++)
	{
		FVector SpawnLoc = GetRandomPointInBox();

		GetWorld()->SpawnActor<AActor>(
			Row->ZombieClass,
			SpawnLoc,
			FRotator::ZeroRotator
		);
	}

	GetWorld()->GetTimerManager().SetTimer(
		WaveTimer,
		this,
		&AHJ_SpawnZombie::NextWave,
		Row->PreparationTime,
		false
	);
}

void AHJ_SpawnZombie::NextWave()
{
	CurrentWave++;
	StartWave(CurrentWave);
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
