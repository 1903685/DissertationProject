// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LatencyCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FramePackage 
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInfo> HitBoxInfo;
};

USTRUCT(BlueprintType)
struct FLagCompensationResult 
{

	GENERATED_BODY()

	UPROPERTY()
	bool bIsHitConfirmed;

	UPROPERTY()
	bool bHeadShot;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEROBE_API ULatencyCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULatencyCompensationComponent();
	friend class AMainCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void FramePackageStory(const FramePackage& Pack, const FColor& Color);
	FLagCompensationResult LagCompensationAlgorithm(
		class AMainCharacter* HitChar, 
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	//Projectile lag compensation
	FLagCompensationResult ProjectileLagCompensation(
		AMainCharacter* HitChar,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);

	//UFUNCTION(Server, Reliable)
	//void Score_Request(
	//	AMainCharacter* HitChar,
	//	const FVector_NetQuantize& TraceStart,
	//	const FVector_NetQuantize& HitLocation,
	//	float HitTime,
	//	class AWeapon* DamageSource
	//);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AMainCharacter* HitChar,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);
	
protected:
	
	virtual void BeginPlay() override;
	void FramePackageSaver(FramePackage& Pack);
	FramePackage InterpFrames(const FramePackage& OlderFrame, const FramePackage& YoungerFrame, float HitTime);
	void CacheBoxPos(AMainCharacter* HitChar, FramePackage& OutFramePack);
	void BoxMovement(AMainCharacter* HitChar, const FramePackage& Pack);
	void ResetBoxes(AMainCharacter* HitChar, const FramePackage& Pack);
	void EnableMeshCollision(AMainCharacter* HitChar, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePack();
	FramePackage GetFrameToCheck(AMainCharacter* HitChar, float HitTime);


	FLagCompensationResult ConfirmHit(
		const FramePackage& Pack,
		AMainCharacter* HitChar,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	//projectiles confirm hit
	FLagCompensationResult ProjConfirmHit(
		const FramePackage& Pack,
		AMainCharacter* HitChar,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);
private:

	UPROPERTY()
	AMainCharacter* Character;

	UPROPERTY()
	class AMainCharPlayerController* Controller;

	TDoubleLinkedList<FramePackage> HistoryFrame;

	UPROPERTY(EditAnywhere)
	float MaxHistoryTime = 4.f;

public:
	
};
