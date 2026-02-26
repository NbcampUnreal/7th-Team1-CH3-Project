#include "HJ_PlayerController.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "HJ_GameState.h"
#include "HJ_Player.h"

AHJ_GameState::AHJ_GameState()
{
	CurrentWave = 0;
	ZombiesRemaining = 0;
	BattleState = EBattleState::Preparation;
}

void AHJ_GameState::SetWave(int32 NewWave)
{
	CurrentWave = NewWave;
}

void AHJ_GameState::SetZombiesRemaining(int32 NewCount)
{
	ZombiesRemaining = NewCount;
}

void AHJ_GameState::SetBattleState(EBattleState NewState)
{
	BattleState = NewState;
}

void AHJ_GameState::UpDateHUD()
{
	if (APlayerController* playerController = GetWorld()->GetFirstPlayerController())
	{
		if (AHJ_PlayerController* HJ_PlayerController = Cast<AHJ_PlayerController>(playerController))
		{
			if (UUserWidget* HUDWidget = HJ_PlayerController->GetHUDWidget())
			{
				if (UTextBlock* WaveText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
				{
					WaveText->SetText(FText::FromString(FString::Printf(TEXT("습격 : %d"),CurrentWave)));
				}
			}
		}
	}
}