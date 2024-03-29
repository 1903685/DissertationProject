// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial   UMETA(DisplayName = "Initial State"),
	EWS_Equipped  UMETA(DisplayName = "Equipped"),
	EWS_Dropped   UMETA(DisplayName = "Dropped"),

	EWS_MAX  UMETA(DisplayName = "DefaultMAX")

};

UCLASS()
class THEROBE_API AWeapon : public AActor
{
	GENERATED_BODY()

public:

	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmunition();
	void ShowPickUpWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void DroppedWeapon();
	void AddAmmunition(int32 AmmoToAdd);
	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsBottom;

	//Auto Fire

	UPROPERTY(EditAnywhere, Category = Comabat)
		float FireDelay = .15;

	UPROPERTY(EditAnywhere, Category = Comabat)
		bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseLagCompensation = false;


	UPROPERTY()
	class AWall* wall;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex

		);


	UPROPERTY()
		class AMainCharacter* OwnerCharacter;

	UPROPERTY()
	class AMainCharPlayerController* OwnerController;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Prop")
		USkeletalMeshComponent* WeaponMesh;


	UPROPERTY(VisibleAnywhere, Category = "Weapon Prop")
		class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Prop")
		EWeaponState WeaponState;

	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Prop")
		class UWidgetComponent* PickUpWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Prop")
		class UAnimationAsset* FireAnim;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABulletShell> BulletShellClass;

	//Zoomed FOV while aiming
	UPROPERTY(EditAnywhere)
		float ZoomFOV = 30.f;

	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
		int32 Ammunition;

	UFUNCTION(Client, Reliable)
	void UpdateAmmoClient(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void AddAmmoClient(int32 AmmoToAdd);

	/*UFUNCTION()
	void OnRep_Ammunition();*/

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MaxCapacity;

	// The number of processed server requiests for Ammo
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammunition;}
	FORCEINLINE int32 GetMagCapacity() const{ return MaxCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }

};
