#include "HJ_PlayerController.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"

AHJ_GameState::AHJ_GameState()
{
	CurrentWave = 1;
	ZombiesRemaining = 0;
	BattleState = EBattleState::Preparation;
}

void AHJ_GameState::SetWave(int32 NewWave)
{
	CurrentWave = NewWave;
	UpDateHUD();
}

void AHJ_GameState::SetZombiesRemaining(int32 NewCount)
{
	ZombiesRemaining = NewCount;
	UpDateHUD();
}

void AHJ_GameState::SetBattleState(EBattleState NewState)
{
	BattleState = NewState;
}

void AHJ_GameState::UpDateHUD()
{
	AHJ_PlayerController* PC = Cast<AHJ_PlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC) return;

	UUserWidget* HUDWidget = PC->GetHUDWidget();
	if (!HUDWidget) return;

	UTextBlock* WaveText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave")));
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(FString::Printf(TEXT("습격 : %d"), CurrentWave)));
	}

}

