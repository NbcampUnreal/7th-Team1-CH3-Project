#include "EquipWeaponMaster.h"
#include "HJ_AiInterface.h"
#include "Engine/Engine.h"

AEquipWeaponMaster::AEquipWeaponMaster()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEquipWeaponMaster::Fire()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
	FVector CameraLocation;
	FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
	FVector End = CameraLocation + (CameraRotation.Vector() * 10000.f);

    FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, End, ECC_Visibility, Params);

    if (bHit)
    {
		AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor->GetClass()->ImplementsInterface(UHJ_AIInterface::StaticClass()))
        {
			UE_LOG(LogTemp, Warning, TEXT("Zomie Hit: %s"), *HitActor->GetName());
			IHJ_AIInterface::Execute_NotifyDeath(HitActor);
        }
    }

    DrawDebugLine(GetWorld(), CameraLocation, End, FColor::Red, false, 2.0f);
}
