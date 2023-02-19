// Fill out your copyright notice in the Description page of Project Settings.


#include "TheRobePlayerState.h"
#include "TheRobe/Character/MainCharacter.h"
#include "TheRobe/PlayerController/MainCharPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATheRobePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATheRobePlayerState, Defeats);
}

void ATheRobePlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMainCharPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ATheRobePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	 Character = Character == nullptr ? Cast<AMainCharacter>(GetPawn()) : Character;
	 if (Character)
	 {
		 Controller = Controller == nullptr ? Cast<AMainCharPlayerController>(Character->Controller) : Controller;
		 if (Controller)
		 {
			 Controller->SetHUDScore(GetScore());
		 }
	 }
}
void ATheRobePlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMainCharPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
void ATheRobePlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMainCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AMainCharPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

