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
	void SetHUDMessagesCountdown(float time);
	void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName MState);
	void MatchHasStarted();
protected:

	virtual void BeginPlay() override;
	void SetMatchTime();
	void PollInit();

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

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	//a function handling a client joining in the middle of the game session
	UFUNCTION(Client, Reliable)
	void ClientJoin(FName State, float WarmUp, float Match, float StartTimer);


private:
	UPROPERTY()
	class AMainCharHUD* MainCharHUD;

	float StartTime = 0.f;
	float MatchTime = 0.f;
	float WarmUpTime = 0.f;
	uint32 Countdown = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharOverlay* CharOverlay;

	bool bCharOverlayInit = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
};
