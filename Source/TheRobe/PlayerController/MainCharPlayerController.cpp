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
