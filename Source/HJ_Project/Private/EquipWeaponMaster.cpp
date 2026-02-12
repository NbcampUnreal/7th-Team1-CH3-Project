#include "EquipWeaponMaster.h"
#include "Engine/Engine.h"

AEquipWeaponMaster::AEquipWeaponMaster()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEquipWeaponMaster::Fire()
{
    UE_LOG(LogTemp, Warning, TEXT("Fire Called"));
}
