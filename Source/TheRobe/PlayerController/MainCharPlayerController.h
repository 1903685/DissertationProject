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
	void SetHUDMatchCountdown(float time);
	void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
protected:

	virtual void BeginPlay() override;
	void SetMatchTime();

	// Time sync between client and server 

	//request the current time of the server
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float clientTime);

	// reports the current server time to the client 
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClient, float ServerReceivedTime);

	// diff between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float SyncFrequency = 5.f;

	float RunningTimeSync = 0.f;

	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
	class AMainCharHUD* MainCharHUD;

	float MatchTime = 120.f;
	uint32 Countdown = 0;
};
