#include "HJ_Player.h"
#include "Camera/CameraComponent.h"
#include "EquipWeaponMaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AHJ_Player::AHJ_Player()
{
	PrimaryActorTick.bCanEverTick = false;

	// 카메라 붐
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	// 카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	// TPS 스타일 회전 설정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AHJ_Player::BeginPlay()
{
	Super::BeginPlay();
}

void AHJ_Player::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Fire();
	}
}