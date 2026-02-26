#include "Gate.h"
#include "Engine/DamageEvents.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
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

	// 시작 시 MaxHealth를 Health로 맞춰주고 싶다면 유지
	Health = MaxHealth;

	// 공격자 수 초기화
	CurrentAttackers = 0;
}

bool AGate::TryRegisterAttacker(AAiEnemyCharacter* Zombie)
{
	
	// 공격자 슬롯 등록 시도
	

	// Gate가 이미 죽었으면 등록 의미 없음
	if (Health <= 0.0f) return false;

	// 좀비가 유효하지 않으면 실패
	if (!IsValid(Zombie)) return false;

	// 슬롯 꽉 찼으면 실패
	if (CurrentAttackers >= MaxAttackers)
	{
		return false;
	}

	// 슬롯 1개 증가
	CurrentAttackers++;

	return true;
}

void AGate::UnregisterAttacker(AAiEnemyCharacter* Zombie)
{
	// 공격자 슬롯 해제
	

	// 안전하게 0 이하로 내려가지 않게
	CurrentAttackers = FMath::Max(0, CurrentAttackers - 1);
}

float AGate::TakeDamage(float DamageAmount,
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

	// 로그 (나중에 지워도 됨)
	FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
	UE_LOG(LogTemp, Warning, TEXT("[Gate] Damaged by: %s | Damage: %.1f | HP: %.1f/%0.1f | Attackers: %d/%d"),
		*CauserName,
		ActualDamage,
		Health,
		MaxHealth,
		CurrentAttackers,
		MaxAttackers
	);

	// Gate 파괴 처리
	if (Health <= 0.0f)
	{
		// 파괴되면 더 이상 슬롯 의미 없으니 0으로 초기화
		CurrentAttackers = 0;

		Destroy();
	}

	return ActualDamage;
}