// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TheRobeGameMode.generated.h"

/**
 * 
 */
UCLASS()
class THEROBE_API ATheRobeGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AMainCharacter* ElimmedChar, class AMainCharPlayerController* VictimController, AMainCharPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedChar, AController* ElimmedController);

};
