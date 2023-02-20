
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TheRobe/HUD/MainCharHUD.h"
#include "TheRobe/Weapon/WeaponTypes.h"
#include "TheRobe/TheRobeTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGHT 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEROBE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	friend class AMainCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void FireButtonActivated(bool bActivated);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();


	void HandleReload();

	int32 ReloadAmount();


private:
	UPROPERTY()
	class AMainCharacter* Character;
	UPROPERTY()
	class AMainCharPlayerController* Controller;
	UPROPERTY()
	class AMainCharHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	bool bFireButtonActivated;

	/**
	/HUD and crosshairs
	*/
	float CrosshairsVelocityFac;
	float CrosshairInAirFac;
	float CrosshairAimFac;
	float CrosshairShootFac;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	//Aiming and FOV

	//FOV when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	//Auto Fire
	FTimerHandle FireTimer;

	bool bCanFire = true;
	
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	//Carried ammo for the current weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CobatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CobatState();

	void UpdateAmmunationVal();

public:	

	
};
