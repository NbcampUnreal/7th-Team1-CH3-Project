#include "HJ_SpawnZombie.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

AHJ_SpawnZombie::AHJ_SpawnZombie()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;

	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHJ_SpawnZombie::BeginPlay()
{
	Super::BeginPlay();
}

FVector AHJ_SpawnZombie::GetRandomPointInBox() const
{
	const FVector Origin = SpawnBox->GetComponentLocation();
	const FVector Extent = SpawnBox->GetScaledBoxExtent();

	return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
}

void AHJ_SpawnZombie::SpawnZombies(int32 Count)
{
	if (!ZombieClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 0; i < Count; i++)
	{
		FVector SpawnLocation = GetRandomPointInBox();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		World->SpawnActor<AActor>(ZombieClass, SpawnLocation, SpawnRotation);
	}
}

void AHJ_SpawnZombie::SpawnItem(TSubclassOf<AActor> ItemClass)
{
	if (!ItemClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLocation = GetRandomPointInBox();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters Params;

	World->SpawnActor<AActor>(ItemClass, SpawnLocation, SpawnRotation, Params);
}

void AHJ_SpawnZombie::SpawnZombies(int32 Count, TSubclassOf<AActor> SelectedZombieClass)
{
	TSubclassOf<AActor> ClassToSpawn = SelectedZombieClass ? SelectedZombieClass : ZombieClass;

	if (!ClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnZombie: 스폰할 좀비 클래스가 지정되지 않았습니다!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 0; i < Count; i++)
	{
		FVector SpawnLocation = GetRandomPointInBox();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		World->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	}
}