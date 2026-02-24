#include "EquipWeaponMaster.h"

#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AEquipWeaponMaster::AEquipWeaponMaster()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(RootComponent);
}

void AEquipWeaponMaster::BeginPlay()
{
	Super::BeginPlay();
}

FVector AEquipWeaponMaster::GetMuzzleLocation() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}


void AEquipWeaponMaster::StartFire()
{
	if (bIsFiring) return;

	bIsFiring = true;

	// 즉시 1발
	Fire();

	// 반복 발사
	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&AEquipWeaponMaster::Fire,
		FireRate,
		true
	);
}


void AEquipWeaponMaster::StopFire()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}


void AEquipWeaponMaster::Fire()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

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

	bool bCamHit = GetWorld()->LineTraceSingleByChannel(
		CamHit,
		CamStart,
		CamEnd,
		ECC_Visibility,
		CamParams
	);

	FVector TargetPoint = bCamHit ? CamHit.ImpactPoint : CamEnd;

	// 총구 → 목표지점
	FVector MuzzleLoc = GetMuzzleLocation();
	FVector ShootDir = (TargetPoint - MuzzleLoc).GetSafeNormal();
	FVector EndPoint = MuzzleLoc + ShootDir * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		MuzzleLoc,
		EndPoint,
		ECC_Visibility,
		Params
	);

	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyDamage(
			Hit.GetActor(),
			CurrentDamage,
			PC,
			this,
			UDamageType::StaticClass()
		);
	}

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), MuzzleLoc, EndPoint, FColor::Red, false, 1.f);
	}
}