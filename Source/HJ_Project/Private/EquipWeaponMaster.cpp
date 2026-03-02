#include "EquipWeaponMaster.h"
#include "HJ_Player.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"//반동/스프레드 각도 계산용

AEquipWeaponMaster::AEquipWeaponMaster()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Muzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(RootComponent);

	//탄약 초기화(탄창 45발)
	MaxAmmoInMag = 45;
	CurrentAmmoInMag = MaxAmmoInMag;

	//반동 패턴
	if (RecoilPattern.Num() == 0)
	{
		RecoilPattern = {
			{0.18f, -0.05f}, 
			{0.19f,  0.04f},
			{0.20f, -0.06f},
			{0.21f,  0.05f},
			{0.22f, -0.04f},
			{0.23f,  0.06f},
			{0.24f, -0.03f},
			{0.25f,  0.04f},
			{0.26f, -0.03f},
			{0.27f,  0.03f},
		};
	}
}

void AEquipWeaponMaster::BeginPlay()
{
	Super::BeginPlay();
}

void AEquipWeaponMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//사격 안 하는 동안 스프레드 천천히 복구
	RecoverSpread(DeltaTime);
}

FVector AEquipWeaponMaster::GetMuzzleLocation() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : GetActorLocation();
}


//탄약
bool AEquipWeaponMaster::CanFire() const
{
	//장전 중에는 발사 불가
	return CurrentAmmoInMag > 0 && !bIsReloading;
}

void AEquipWeaponMaster::ConsumeAmmo()
{
	CurrentAmmoInMag = FMath::Max(0, CurrentAmmoInMag - 1);
}

//;리로딩
void AEquipWeaponMaster::Reload()
{
	// 예비탄 무한이라 그냥 탄창만 채움
	CurrentAmmoInMag = MaxAmmoInMag;
}

//장전 완료처리
void AEquipWeaponMaster::FinishReload()
{
	// 예비탄 무한이라 그냥 탄창만 채움
	CurrentAmmoInMag = MaxAmmoInMag;

	bIsReloading = false;
}


void AEquipWeaponMaster::ResetBurst()
{
	ShotsFiredInBurst = 0;
}

void AEquipWeaponMaster::IncreaseSpread()
{
	CurrentSpreadDeg = FMath::Min(MaxSpreadDeg, CurrentSpreadDeg + SpreadIncreasePerShotDeg);
}

void AEquipWeaponMaster::RecoverSpread(float DeltaTime)
{
	//발사도중에도 반동 서서히 복구
	const float Target = BaseSpreadDeg;
	CurrentSpreadDeg = FMath::FInterpTo(CurrentSpreadDeg, Target, DeltaTime, SpreadRecoverySpeed);
}


//반동
void AEquipWeaponMaster::ApplyRecoil(APlayerController* PC)
{
	if (!PC) return;

	float Pitch = 0.f;
	float Yaw = 0.f;

	if (RecoilPattern.Num() > 0)
	{
		const int32 Index = FMath::Clamp(ShotsFiredInBurst, 0, RecoilPattern.Num() - 1);
		Pitch = RecoilPattern[Index].Pitch;
		Yaw = RecoilPattern[Index].Yaw;

		//총을 계속쏘면 반동크게 증가(상한선 지정해둠)
		if (ShotsFiredInBurst >= RecoilOverflowStartShot)
		{
			const int32 Overflow = ShotsFiredInBurst - RecoilOverflowStartShot + 1;

			
			const float GentleMul = 1.0f + (RecoilOverflowSlope * (float)Overflow);

			Pitch *= GentleMul;
			Yaw *= GentleMul;
		}

		// 반동 상한선. 계속 쏴도 통제 가능한 범위 유지
		Pitch = FMath::Clamp(Pitch, 0.0f, MaxRecoilPitchPerShot);
		Yaw = FMath::Clamp(Yaw, -MaxRecoilYawPerShot, MaxRecoilYawPerShot);
	}

	// 컨트롤러 입력으로 반동 적용
	PC->AddPitchInput(-Pitch);
	PC->AddYawInput(Yaw);

	ShotsFiredInBurst++;
}

void AEquipWeaponMaster::Fire()
{
	
	if (!GetWorld()) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	//탄 없으면 발사 막기 + 연사 중이면 타이머도 끊기
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] No Ammo in Mag. Reload!"));
		StopFire();
		ResetBurst();
		return;
	}

	ConsumeAmmo();     //발사 시 탄 1발 소모
	IncreaseSpread();  //연사할수록 스프레드 누적

	//총기사운드
	if (FireSound2D)
	{
		UGameplayStatics::PlaySound2D(this, FireSound2D);
	}


	//총구화염
	if (Muzzle && MuzzleFlashNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashNiagara,
			Muzzle,                 // ArrowComponent에 붙임
			NAME_None,              // 소켓 없음 (Arrow라서)
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true                    // 자동 삭제
		);
	}



	// 0) 현재 무기 데미지 체크
	UE_LOG(LogTemp, Warning, TEXT("[Fire] CurrentDamage=%.1f Ammo=%d/%d"), CurrentDamage, CurrentAmmoInMag, MaxAmmoInMag);

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

	// 2) 총구에서 AimPoint 방향으로 실제 발사
	const FVector MuzzleLoc = GetMuzzleLocation();
	FVector ShotDir = (AimPoint - MuzzleLoc).GetSafeNormal();

	//스프레드 적용 완전 랜덤이 아니라 현재 스프레드만큼만 퍼지게
	if (CurrentSpreadDeg > KINDA_SMALL_NUMBER)
	{
		const float ConeHalfAngleRad = FMath::DegreesToRadians(CurrentSpreadDeg);
		ShotDir = FMath::VRandCone(ShotDir, ConeHalfAngleRad);
	}

	const FVector ShotEnd = MuzzleLoc + (ShotDir * TraceDistance);

	FHitResult ShotHit;
	FCollisionQueryParams ShotParams(SCENE_QUERY_STAT(ShotTrace), true);
	ShotParams.AddIgnoredActor(this);
	ShotParams.AddIgnoredActor(GetOwner());
	ShotParams.bReturnPhysicalMaterial = true;
	ShotParams.bTraceComplex = true;

	const bool bShotHit = GetWorld()->LineTraceSingleByChannel(
		ShotHit, MuzzleLoc, ShotEnd, ECC_Visibility, ShotParams);

	UE_LOG(LogTemp, Warning, TEXT("[Fire] bShotHit=%d"), bShotHit ? 1 : 0);
	if (bShotHit && ShotHit.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] HitActor=%s"), *ShotHit.GetActor()->GetName());
	}

	APawn* InstigatorPawn = GetInstigator();

	UE_LOG(LogTemp, Warning, TEXT("[Fire] Instigator=%s Class=%s"),
		InstigatorPawn ? *InstigatorPawn->GetName() : TEXT("NULL"),
		InstigatorPawn ? *InstigatorPawn->GetClass()->GetName() : TEXT("NULL"));

	// 기존 화면 흔들림은 그대로 유지
	if (AHJ_Player* P = Cast<AHJ_Player>(InstigatorPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] Cast to AHJ_Player SUCCESS (Instigator)"));
		P->AddRecoilImpulse();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] Cast to AHJ_Player FAILED (Instigator)"));
	}

	//무기에서 컨트롤러 입력으로 추가 적용
	ApplyRecoil(PC);

	// Debug (원하면 끄기)
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

	// 4) 데미지가 0이면 종료
	if (CurrentDamage <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] CurrentDamage is 0. Set it from DT_Weapon in BP."));
		return;
	}

	// 5) ApplyPointDamage로 좀비 TakeDamage로 전달
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
	//탄 없으면 연사 시작 자체를 막음
	if (!CanFire())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StartFire] No Ammo. Reload!"));
		return;
	}

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

	ResetBurst(); //버튼 뗐으면 다음 연사는 패턴 처음부터 시작
}