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

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;




	//HP초기값
	MaxHP = 100.0f;
	CurrentHP = MaxHP;
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

//데메지 정의
float AHJ_Player::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	if (ActualDamage > 0.0f)
	{
		//체력감소
		CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);
		//로그표시임
		UE_LOG(LogTemp, Warning, TEXT("HP: %f"), CurrentHP);

		if (CurrentHP <= 0.0f)
		{
			//0이되면 게임오버 로그
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