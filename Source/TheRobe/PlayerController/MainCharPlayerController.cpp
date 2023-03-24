// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharPlayerController.h"
#include "TheRobe/HUD/MainCharHUD.h"
#include "TheRobe/HUD/CharOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TheRobe/Character/MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TheRobe/GameMode/TheRobeGameMode.h"
#include "TheRobe/HUD/Messages.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "TheRobe/PlayerState/TheRobePlayerState.h"

void AMainCharPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	MainCharHUD = Cast<AMainCharHUD>(GetHUD());
	ServerCheckMatchState();
}
void AMainCharPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainCharPlayerController, MatchState);

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
	PollInit();
	CheckPing(DeltaTime);
	
}
void AMainCharPlayerController::CheckPing(float DeltaTime)
{
	PingWarningRunningTime += DeltaTime;
	if (PingWarningRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > PingThreshold)
			{
				PingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}
		PingWarningRunningTime = 0.f;
	}
	bool bPingWarningAnimPlaying =
		MainCharHUD && MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->PingWarningAnimation &&
		MainCharHUD->CharacterOverlay->IsAnimationPlaying(MainCharHUD->CharacterOverlay->PingWarningAnimation);
	if (bPingWarningAnimPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > PingWarningDuration)
		{
			StopPingWarning();
		}
	}
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
void AMainCharPlayerController::PingWarning()
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;

	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->PingWarningImage
		&& MainCharHUD->CharacterOverlay->PingWarningAnimation;

	if (bIsHUDValid)
	{
		MainCharHUD->CharacterOverlay->PingWarningImage->SetOpacity(1.f);
		MainCharHUD->CharacterOverlay->PlayAnimation(
			MainCharHUD->CharacterOverlay->PingWarningAnimation,
			0.f,
			5);
	}
}
void AMainCharPlayerController::StopPingWarning()
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;

	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->CharacterOverlay &&
		MainCharHUD->CharacterOverlay->PingWarningImage
		&& MainCharHUD->CharacterOverlay->PingWarningAnimation;

	if (bIsHUDValid)
	{
		MainCharHUD->CharacterOverlay->PingWarningImage->SetOpacity(0.f);
		if (MainCharHUD->CharacterOverlay->IsAnimationPlaying(MainCharHUD->CharacterOverlay->PingWarningAnimation))
		{
			MainCharHUD->CharacterOverlay->StopAnimation(MainCharHUD->CharacterOverlay->PingWarningAnimation);
		}
	}
}

void AMainCharPlayerController::ClientJoin_Implementation(FName State, float WarmUp, float Match, float StartTimer)
{
	WarmUpTime = WarmUp;
	MatchTime = Match;
	StartTime = StartTimer;
	MatchState = State;
	OnMatchStateSet(MatchState);

	if (MainCharHUD && MatchState == MatchState::WaitingToStart)
	{
		MainCharHUD->AddMessages();
	}
}
void AMainCharPlayerController::ServerCheckMatchState_Implementation()
{
	ATheRobeGameMode* GameMode = Cast<ATheRobeGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmupTimer;
		MatchTime = GameMode->MatchTimer;
		StartTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoin(MatchState, WarmUpTime, MatchTime, StartTime);

		if (MainCharHUD && MatchState == MatchState::WaitingToStart) 
		{
			MainCharHUD->AddMessages();
		}
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
	else
	{
	    bCharOverlayInit = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bCharOverlayInit = true;
		HUDScore = Score;
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
	else
	{
		bCharOverlayInit = true;
		HUDDefeats = Defeats;
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
void AMainCharPlayerController::SetHUDMessagesCountdown(float time)
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	bool bIsHUDValid = MainCharHUD &&
		MainCharHUD->Messages &&
		MainCharHUD->Messages->WarmupTimer;

	if (bIsHUDValid)
	{
		int32 Min = FMath::FloorToInt(time / 60);
		int32 Sec = time - Min * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Min, Sec);
		MainCharHUD->Messages->WarmupTimer->SetText(FText::FromString(CountdownText));
	}
}
void AMainCharPlayerController::SetMatchTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmUpTime - GetServerTime() + StartTime;
	}
	else if (MatchState == MatchState::InProgress) 
	{
		TimeLeft = WarmUpTime + MatchTime - GetServerTime() + StartTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (Countdown != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDMessagesCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}

	}
	Countdown = SecondsLeft;
}

void AMainCharPlayerController::PollInit()
{
	if (CharOverlay == nullptr)
	{
		if (MainCharHUD && MainCharHUD->CharacterOverlay)
		{
			CharOverlay = MainCharHUD->CharacterOverlay;
			if (CharOverlay)
			{
				SetHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}


void AMainCharPlayerController::ServerRequestServerTime_Implementation(float clientTime)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(clientTime, ServerTimeOfReceipt);
}

void AMainCharPlayerController::ClientReportServerTime_Implementation(float TimeOfClient, float ServerReceivedTime)
{
	float RTT = GetWorld()->GetTimeSeconds() - TimeOfClient;
	SingleTTripTime = 0.5 * RTT;
	float ServerTime = ServerReceivedTime + SingleTTripTime;
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

void AMainCharPlayerController::OnMatchStateSet(FName MState)
{
	MatchState = MState;
	if (MatchState == MatchState::InProgress)
	{
		MatchHasStarted();
	}
}
void AMainCharPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		MatchHasStarted();
	}
}
void AMainCharPlayerController::MatchHasStarted()
{
	MainCharHUD = MainCharHUD == nullptr ? Cast<AMainCharHUD>(GetHUD()) : MainCharHUD;
	if (MainCharHUD)
	{
		MainCharHUD->AddCharaOverlay();
		if (MainCharHUD->Messages)
		{
			MainCharHUD->Messages->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
