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

	// 0) 현재 무기 데미지 체크 (BP에서 DT_Weapon로 세팅돼야 함)
	UE_LOG(LogTemp, Warning, TEXT("[Fire] CurrentDamage=%.1f"), CurrentDamage);

	// 1) 카메라 기준 조준점(AimPoint) 구하기
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector CamForward = CamRot.Vector();
	const FVector CamStart = CamLoc + CamForward * 30.f;   // 30cm 앞에서 시작 (50도 OK)
	const FVector CamEnd = CamStart + CamForward * TraceDistance;

	FHitResult CamHit;
	FCollisionQueryParams CamParams(SCENE_QUERY_STAT(CameraTrace), true);
	CamParams.AddIgnoredActor(this);
	CamParams.AddIgnoredActor(GetOwner());
	const bool bCamHit = GetWorld()->LineTraceSingleByChannel(
		CamHit, CamStart, CamEnd, ECC_Visibility, CamParams);

	const FVector AimPoint = bCamHit ? CamHit.ImpactPoint : CamEnd;

	// 2) 총구에서 AimPoint 방향으로 실제 발사(2단 트레이스)
	const FVector MuzzleLoc = GetMuzzleLocation();
	const FVector ShotDir = (AimPoint - MuzzleLoc).GetSafeNormal();
	const FVector ShotEnd = MuzzleLoc + (ShotDir * TraceDistance);

	FHitResult ShotHit;
	FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(ShotTrace), true);
	ShotParams.AddIgnoredActor(this);
	ShotParams.AddIgnoredActor(GetOwner());

	//헤드샷 판정 정보
	ShotParams.bReturnPhysicalMaterial = true;
	ShotParams.bTraceComplex = true;

	const bool bShotHit = GetWorld()->LineTraceSingleByChannel(
		ShotHit, MuzzleLoc, ShotEnd, ECC_Visibility, ShotParams);

	UE_LOG(LogTemp, Warning, TEXT("[Fire] bShotHit=%d"), bShotHit ? 1 : 0);
	if (bShotHit && ShotHit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] HitActor=%s"), *ShotHit.GetActor()->GetName());
	}

	// Debug (원하면 끄기)
	if (bDrawDebug)
	{
		if (bDrawCameraDebug)
		{
			DrawDebugLine(GetWorld(), CamLoc, AimPoint, FColor::Red, false, 0.6f, 0, 1.0f);
		}

		if (bDrawMuzzleDebug)
		{
			const FVector RealEnd = bShotHit ? ShotHit.ImpactPoint : ShotEnd;
			DrawDebugLine(GetWorld(), MuzzleLoc, RealEnd, FColor::Green, false, 0.6f, 0, 1.5f);

			if (bShotHit)
			{
				DrawDebugSphere(GetWorld(), ShotHit.ImpactPoint, 8.f, 12, FColor::Yellow, false, 0.8f);
			}
		}
	}

	// 3) 히트가 없으면 종료
	if (!bShotHit) return;

	AActor* HitActor = ShotHit.GetActor();
	if (!HitActor) return;

	//관문 보호
	if (HitActor->ActorHasTag(TEXT("Gate")))
	{
		UE_LOG(LogTemp, Warning, TEXT("관문명중!"));
		return;
	}


	// 4) 데미지가 0이면 종료 (BP에서 DT_Weapon 값이 세팅 안 된 상태)
	if (CurrentDamage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] CurrentDamage is 0. Set it from DT_Weapon in BP."));
		return;
	}

	// 5) ApplyPointDamage로 좀비 TakeDamage로 전달
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

	UE_LOG(LogTemp, Warning, TEXT("[Fire] ApplyPointDamage -> %.1f to %s"), CurrentDamage, *HitActor->GetName());
}