#include "Gate.h"
#include "Engine/DamageEvents.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "HJ_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "AI/AiEnemyCharacter.h"

AGate::AGate()
{
	PrimaryActorTick.bCanEverTick = false;
	// Gate는 Tick 필요 없음

	// 메쉬 컴포넌트 생성 (루트)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
	// Gate는 기본적으로 다 막아도 됨

	// HP 위젯 컴포넌트 생성
	HPWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidget"));
	HPWidgetComp->SetupAttachment(RootComponent);

	HPWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	// 화면 UI로 표시

	HPWidgetComp->SetDrawAtDesiredSize(true);
	// 위젯 사이즈 자동

	// 좀비가 찾을 수 있도록 태그 부여
	Tags.Add(FName("Gate"));
}

void AGate::BeginPlay()
{
	Super::BeginPlay();

	// 시작 체력 확정
	Health = MaxHealth;

	// 공격자 초기화
	RegisteredAttackers.Empty();
	CurrentAttackers = 0;
}

bool AGate::TryRegisterAttacker(AAiEnemyCharacter* Zombie)
{
	// Gate가 이미 죽었으면 등록 의미 없음
	if (Health <= 0.0f) return false;

	// 좀비가 유효하지 않으면 실패
	if (!IsValid(Zombie)) return false;

	// 이미 등록된 좀비면 성공 처리(중복 증가 방지)
	if (RegisteredAttackers.Contains(Zombie))
	{
		return true;
	}

	// 슬롯 꽉 찼으면 실패
	if (RegisteredAttackers.Num() >= MaxAttackers)
	{
		return false;
	}

	// 등록
	RegisteredAttackers.Add(Zombie);
	CurrentAttackers = RegisteredAttackers.Num();

	return true;
}

void AGate::UnregisterAttacker(AAiEnemyCharacter* Zombie)
{
	if (!IsValid(Zombie)) return;

	RegisteredAttackers.Remove(Zombie);
	CurrentAttackers = RegisteredAttackers.Num();
}

float AGate::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	// 실제 데미지가 없으면 종료
	if (ActualDamage <= 0.0f) return 0.0f;

	// 체력 감소
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

	// Gate 파괴 처리
	if (Health <= 0.0f)
	{
		// 파괴되면 슬롯 초기화
		RegisteredAttackers.Empty();
		CurrentAttackers = 0;

		Destroy();

		AHJ_GameMode* GameMode = Cast<AHJ_GameMode>(UGameplayStatics::GetGameMode(this));
		if (GameMode)
		{
			GameMode->HandleDefeat();
		}
	}

	return ActualDamage;
}