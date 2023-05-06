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
	ATheRobeGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AMainCharacter* ElimmedChar, class AMainCharPlayerController* VictimController, AMainCharPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedChar, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTimer = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTimer = 120.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountDownTimer = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return  CountDownTimer; }

};
