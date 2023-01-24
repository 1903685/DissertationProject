// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 playersNumber = GameState.Get()->PlayerArray.Num();

	if (playersNumber == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			//travel seamlessly 
			bUseSeamlessTravel = true;
			//travl to Travel Map as soon as the player's number reaches 2
			World->ServerTravel(FString("/Game/Maps/TravelMap?listen"));
		}
	}

}
