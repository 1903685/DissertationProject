// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TheRobe/TheRobeTypes/TurningInPlace.h"
#include "TheRobe/Interfaces/CrosshairsInteraction.h"
#include "Components/TimelineComponent.h"
#include "TheRobe/TheRobeTypes/CombatState.h"
#include "MainCharacter.generated.h"


UCLASS()
class THEROBE_API AMainCharacter : public ACharacter, public ICrosshairsInteraction
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents();
	void PlayFireMontage(bool bIsAiming);
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	virtual void OnRep_ReplicatedMovement() override;
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateHealth();

	void MoveForward(float _val);
	void MoveRight(float _val);
	void Turn(float _val);
	void LookUP(float _val);
	void EquipButtonActivated();
	void CrouchButtonActivated();
	void ReloadButtonActivated();
	void AimButtonActivated();
	void AimButtonReleased();
	void AimOffset(float dt);
	void CalculatePitch_AO();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonActivated();
	void FireButtonDeactivated();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	//poll and init HUD
	void PollInit();

	//box component
	// Hit boxes used for lag compensation algorithm
	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* rucksack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_left;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_right;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_left;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_right;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_left;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_right;


private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* CameraFollow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);


	//Main character components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class ULatencyCompensationComponent* LCC;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonActivated();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRot;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float dt);

	//Animation Montages
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;


	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;


	void HideCameraWhenCharacterClose();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;

	float TurnTreshhold = 0.5f;
	FRotator ProxyRotLastFrame;
	FRotator ProxyRot;
	float ProxyYaw;
	float TimeSinceLastMovementRep;
	float CalculateSpeed();

	//Character Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AMainCharPlayerController* MainCharPlayerController;

	bool bIsElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	//Disovle effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DisolveTimeLine;
	FOnTimelineFloat DisolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DisolveCurve;
	
	UFUNCTION()
	void UpdateMaterial(float DisolveVal);
	void StartMaterial();

	//change at run time
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynDisolveMaterialInstance;

	//set on the Blueprint
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DisolveMaterialInstance;

	
	class ATheRobePlayerState* PlayerState;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return CameraFollow; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bIsElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	bool IsReloadingLocally();
	FORCEINLINE ULatencyCompensationComponent* GetLatencyCompensation() const { return LCC; }
};
