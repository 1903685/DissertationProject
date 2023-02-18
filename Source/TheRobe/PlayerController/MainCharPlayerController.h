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

protected:

	virtual void BeginPlay() override;

private:
	class AMainCharHUD* MainCharHUD;

	
};
