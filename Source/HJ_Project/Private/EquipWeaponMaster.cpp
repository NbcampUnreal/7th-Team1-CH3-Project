#include "EquipWeaponMaster.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "HJ_AiInterface.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/Engine.h"

AEquipWeaponMaster::AEquipWeaponMaster()
{
    PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(Root);
}

FVector AEquipWeaponMaster::GetMuzzleLocation() const
{
	if (Muzzle) return Muzzle->GetComponentLocation();

	return GetActorLocation();
}

void AEquipWeaponMaster::Fire()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// =========================
	// 1) 카메라 기준 조준점(AimPoint) 구하기
	// =========================
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

	// =========================
	// 2) 총구에서 AimPoint 방향으로 실제 발사(2단 트레이스)
	// =========================
	const FVector MuzzleLoc = GetMuzzleLocation();
	const FVector ShotDir = (AimPoint - MuzzleLoc).GetSafeNormal();
	const FVector ShotEnd = MuzzleLoc + (ShotDir * TraceDistance);

	FHitResult ShotHit;
	FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(ShotTrace), true);
	ShotParams.AddIgnoredActor(this);
	ShotParams.AddIgnoredActor(GetOwner());

	const bool bShotHit = GetWorld()->LineTraceSingleByChannel(
		ShotHit, MuzzleLoc, ShotEnd, ECC_Visibility, ShotParams);

	// =========================
	// Debug
	// =========================
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

	// =========================
	// 3) 히트 처리 (현재는 즉사 NotifyDeath)
	// =========================
	if (bShotHit)
	{
		AActor* HitActor = ShotHit.GetActor();
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UHJ_AIInterface::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Zombie Hit: %s"), *HitActor->GetName());
			IHJ_AIInterface::Execute_NotifyDeath(HitActor);
		}
	}
}
