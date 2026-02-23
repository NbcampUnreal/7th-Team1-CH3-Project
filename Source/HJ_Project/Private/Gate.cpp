#include "Gate.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"

AGate::AGate()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트로 메쉬 설정
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	// UI 컴포넌트 생성 및 부착
	HPWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidget"));
	HPWidgetComp->SetupAttachment(RootComponent);

	
	HPWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HPWidgetComp->SetDrawAtDesiredSize(true); 

	// 좀비 인식을 위한 태그
	Tags.Add(FName("Gate"));
}

void AGate::BeginPlay()
{
	Super::BeginPlay();
	MaxHealth = Health;
}

float AGate::TakeDamage(float DamageAmount, 
	struct FDamageEvent const& DamageEvent, 
	AController* EventInstigator,
	AActor* DamageCauser)
{
	
	float ActualDamage = Super::TakeDamage(DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	if (ActualDamage > 0.0f)
	{
		Health -= ActualDamage;

		if (Health <= 0.0f)
		{
			
			Destroy();
		}
	}
	return ActualDamage;
}