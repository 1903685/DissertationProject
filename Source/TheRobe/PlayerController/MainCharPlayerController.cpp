// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharPlayerController.h"
#include "TheRobe/HUD/MainCharHUD.h"
#include "TheRobe/HUD/CharOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TheRobe/Character/MainCharacter.h"

void AMainCharPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MainCharHUD = Cast<AMainCharHUD>(GetHUD());
}
void AMainCharPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMainCharacter* MainCharacter = Cast<AMainCharacter>(InPawn);
	if (MainCharacter)
	{
		SetHealth(MainCharacter->GetHealth(), MainCharacter->GetMaxHealth());
	}
}

void AMainCharPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetMatchTime();
	CheckTimeSync(DeltaTime);
}
void AMainCharPlayerController::CheckTimeSync(float DeltaTime)
{
	RunningTimeSync += DeltaTime;
	if (IsLocalController() && RunningTimeSync > SyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		RunningTimeSync = 0.f;
	}
}
void AMainCharPlayerController::SetHealth(float Health, float MaxHealth)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	
	bool bIsHUDValid = MainCharHUD && 
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->HealthBar
		&& MainCharHUD->CharacterOverlay->HealthText;

	if (bIsHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		MainCharHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MainCharHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AMainCharPlayerController::SetHUDScore(float Score)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->ScoreAmount;

	if (bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MainCharHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void AMainCharPlayerController::SetHUDDefeats(int32 Defeats)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->DefeatsAmount;

	if (bIsHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MainCharHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}

}

void AMainCharPlayerController::SetHudWeaponAmmo(int32 Ammunition)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammunition);
		MainCharHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}

}

void AMainCharPlayerController::SetHudCarriedAmmo(int32 Ammunition)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammunition);
		MainCharHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMainCharPlayerController::SetHUDMatchCountdown(float time)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->MatchCountdownText;

	if (bIsHUDValid)
	{
		int32 Min = FMath::FloorToInt(time / 60);
		int32 Sec = time - Min * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Min, Sec);
		MainCharHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}
void AMainCharPlayerController::SetMatchTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (Countdown != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	Countdown = SecondsLeft;
}

void AMainCharPlayerController::ServerRequestServerTime_Implementation(float clientTime)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(clientTime, ServerTimeOfReceipt);
}

void AMainCharPlayerController::ClientReportServerTime_Implementation(float TimeOfClient, float ServerReceivedTime)
{
	float RTT = GetWorld()->GetTimeSeconds() - TimeOfClient;
	float ServerTime = ServerReceivedTime + (0.5 * RTT);
	ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds();
}

float AMainCharPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMainCharPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
