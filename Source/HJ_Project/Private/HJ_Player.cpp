#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AHJ_Player::AHJ_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AHJ_Player::BeginPlay()
{
	Super::BeginPlay();
	if (WeaponClass)
	{
		CurrentWeapon = GetWorld()->SpawnActor<AEquipWeaponMaster>(WeaponClass);
		CurrentWeapon->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			TEXT("hand_rSocket"));

		CurrentWeapon->SetOwner(this);
	}
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