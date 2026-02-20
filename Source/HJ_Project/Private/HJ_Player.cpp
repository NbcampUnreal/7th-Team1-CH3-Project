#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"

AHJ_Player::AHJ_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	// 카메라 컴포넌트
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	// 카메라 위치 조정
	CameraBoom->TargetArmLength = 420.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 70.f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.f;

	// 카메라 회전
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;

	// FollowCamera 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	// 캐릭터 회전 설정
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	MaxHP = 100.0f;
	CurrentHP = MaxHP;
}

void AHJ_Player::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponClass)
	{
		CurrentWeapon = GetWorld()->SpawnActor<AEquipWeaponMaster>(WeaponClass);
		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("hand_rSocket")
			);
			CurrentWeapon->SetOwner(this);
		}
	}
}

void AHJ_Player::SetAimMode(bool bAim)
{
	bIsAiming = bAim;
}

float AHJ_Player::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{
		CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);

		UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHP);

		if (CurrentHP <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("YOU DIE YANG!"));
		}
	}

	return ActualDamage;
}

void AHJ_Player::StartFire()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire Input"));

	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon OK"));
		CurrentWeapon->Fire();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentWeapon NULL"));
	}
}