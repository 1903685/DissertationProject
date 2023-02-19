// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainCharPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THEROBE_API AMainCharPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHudWeaponAmmo(int32 Ammunition);
	void SetHudCarriedAmmo(int32 Ammunition);
	void OnPossess(APawn* InPawn) override;
protected:

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class AMainCharHUD* MainCharHUD;

	
};
