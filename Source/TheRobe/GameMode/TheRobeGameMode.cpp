// Fill out your copyright notice in the Description page of Project Settings.


#include "TheRobeGameMode.h"
#include "TheRobe/Character/MainCharacter.h"
#include "TheRobe/PlayerController/MainCharPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ATheRobeGameMode::PlayerEliminated(AMainCharacter* ElimmedChar, AMainCharPlayerController* VictimController, AMainCharPlayerController* AttackerController)
{
	if (ElimmedChar)
	{
		ElimmedChar->Elim();
	}
}

void ATheRobeGameMode::RequestRespawn(ACharacter* ElimmedChar, AController* ElimmedController)
{
	if (ElimmedChar)
	{
		ElimmedChar->Reset();
		ElimmedChar->Destroy();
	}

	if (ElimmedController)
	{
		//Respawn the player at a Random player Start pos
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), Actors);
		int32 Selection = FMath::RandRange(0, Actors.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, Actors[Selection]);
	}
}
