#include "EquipWeaponMaster.h"

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
}

FVector AEquipWeaponMaster::GetMuzzleLocation() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}

void AEquipWeaponMaster::Fire()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector CamEnd = CamLoc + (CamRot.Vector() * TraceDistance);

	FHitResult CamHit;
	FCollisionQueryParams CamParams(SCENE_QUERY_STAT(CameraTrace), true);
	CamParams.AddIgnoredActor(this);
	CamParams.AddIgnoredActor(GetOwner());

	const bool bCamHit = GetWorld()->LineTraceSingleByChannel(
		CamHit, CamLoc, CamEnd, ECC_Visibility, CamParams);

	const FVector AimPoint = bCamHit ? CamHit.ImpactPoint : CamEnd;

	const FVector MuzzleLoc = GetMuzzleLocation();
	const FVector ShotDir = (AimPoint - MuzzleLoc).GetSafeNormal();
	const FVector ShotEnd = MuzzleLoc + (ShotDir * TraceDistance);

	FHitResult ShotHit;
	FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(ShotTrace), true);
	ShotParams.AddIgnoredActor(this);
	ShotParams.AddIgnoredActor(GetOwner());

	const bool bShotHit = GetWorld()->LineTraceSingleByChannel(
		ShotHit, MuzzleLoc, ShotEnd, ECC_Visibility, ShotParams);

	if (bDrawDebug)
	{
		if (bDrawCameraDebug)
		{
			DrawDebugLine(GetWorld(), CamLoc, AimPoint, FColor::Red, false, 0.6f, 0, 1.0f);
		}

		if (bDrawMuzzleDebug)
		{
			const FVector RealEnd = bShotHit ? ShotHit.ImpactPoint : ShotEnd;
			DrawDebugLine(GetWorld(), MuzzleLoc, RealEnd, FColor::Green, false, 0.6f, 0, 1.0f);
		}
	}

	if (!bShotHit) return;

	AActor* HitActor = ShotHit.GetActor();
	if (!HitActor) return;

	// ✅ BP에서 CurrentDamage를 세팅 안 했으면 데미지 0이라 아무 일도 안 하게 처리
	if (CurrentDamage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] CurrentDamage is 0. Set it from DT_Weapon in BP."));
		return;
	}

	AController* InstigatorController = nullptr;
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		InstigatorController = OwnerPawn->GetController();
	}

	UGameplayStatics::ApplyPointDamage(
		HitActor,
		CurrentDamage,
		ShotDir,
		ShotHit,
		InstigatorController,
		this,
		UDamageType::StaticClass()
	);
}