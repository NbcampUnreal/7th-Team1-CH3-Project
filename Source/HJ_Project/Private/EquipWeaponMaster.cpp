#include "EquipWeaponMaster.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"

AEquipWeaponMaster::AEquipWeaponMaster()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(RootComponent);

	CurrentAmmo = MaxAmmo;
	CurrentMagazineAmmo = MagazineSize;
}

FVector AEquipWeaponMaster::GetMuzzleLocation() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}

/* ================= Reload ================= */

void AEquipWeaponMaster::Reload()
{
	if (bIsReloading) return;
	if (CurrentAmmo <= 0) return;
	if (CurrentMagazineAmmo >= MagazineSize) return;

	bIsReloading = true;

	StopFire();

	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AEquipWeaponMaster::FinishReload,
		ReloadTime,
		false
	);
}

void AEquipWeaponMaster::FinishReload()
{
	int32 NeededAmmo = MagazineSize - CurrentMagazineAmmo;
	int32 AmmoToReload = FMath::Min(NeededAmmo, CurrentAmmo);

	CurrentMagazineAmmo += AmmoToReload;
	CurrentAmmo -= AmmoToReload;

	bIsReloading = false;

	UE_LOG(LogTemp, Warning, TEXT("[Reload] Magazine=%d / Reserve=%d"),
		CurrentMagazineAmmo, CurrentAmmo);
}

/* ================= Fire ================= */

void AEquipWeaponMaster::Fire()
{
	if (bIsReloading) return;

	if (CurrentMagazineAmmo <= 0)
	{
		Reload();
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	CurrentMagazineAmmo--;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector CamForward = CamRot.Vector();
	const FVector CamStart = CamLoc + CamForward * 30.f;
	const FVector CamEnd = CamStart + CamForward * TraceDistance;

	FHitResult CamHit;
	FCollisionQueryParams CamParams(SCENE_QUERY_STAT(CameraTrace), true);
	CamParams.AddIgnoredActor(this);
	CamParams.AddIgnoredActor(GetOwner());

	const bool bCamHit = GetWorld()->LineTraceSingleByChannel(
		CamHit, CamStart, CamEnd, ECC_Visibility, CamParams);

	const FVector AimPoint = bCamHit ? CamHit.ImpactPoint : CamEnd;

	const FVector MuzzleLoc = GetMuzzleLocation();
	const FVector ShotDir = (AimPoint - MuzzleLoc).GetSafeNormal();
	const FVector ShotEnd = MuzzleLoc + (ShotDir * TraceDistance);

	FHitResult ShotHit;
	FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(ShotTrace), true);
	ShotParams.AddIgnoredActor(this);
	ShotParams.AddIgnoredActor(GetOwner());
	ShotParams.bReturnPhysicalMaterial = true;
	ShotParams.bTraceComplex = true;

	const bool bShotHit = GetWorld()->LineTraceSingleByChannel(
		ShotHit, MuzzleLoc, ShotEnd, ECC_Visibility, ShotParams);

	if (bDrawDebug)
	{
		if (bDrawCameraDebug)
			DrawDebugLine(GetWorld(), CamLoc, AimPoint, FColor::Red, false, 0.5f);

		if (bDrawMuzzleDebug)
		{
			FVector RealEnd = bShotHit ? ShotHit.ImpactPoint : ShotEnd;
			DrawDebugLine(GetWorld(), MuzzleLoc, RealEnd, FColor::Green, false, 0.5f);
		}
	}

	if (!bShotHit) return;

	AActor* HitActor = ShotHit.GetActor();
	if (!HitActor) return;

	if (HitActor->ActorHasTag(TEXT("Gate")))
		return;

	if (CurrentDamage <= 0.f) return;

	AController* InstigatorController = nullptr;
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		InstigatorController = OwnerPawn->GetController();

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		CurrentDamage,
		ShotDir,
		ShotHit,
		InstigatorController,
		this,
		UDamageType::StaticClass()
	);

	UE_LOG(LogTemp, Warning, TEXT("[Fire] Magazine=%d / Reserve=%d"),
		CurrentMagazineAmmo, CurrentAmmo);
}

void AEquipWeaponMaster::StartFire()
{
	if (bIsReloading) return;

	if (FireRate <= 0.f)
	{
		Fire();
		return;
	}

	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&AEquipWeaponMaster::Fire,
		FireRate,
		true,
		0.0f
	);
}

void AEquipWeaponMaster::StopFire()
{
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}