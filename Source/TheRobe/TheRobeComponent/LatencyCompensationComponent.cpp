#include "LatencyCompensationComponent.h"
#include "TheRobe/Character/MainCharacter.h"
#include "TheRobe/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TheRobe/TheRobe.h"
#include "Wall.h"

ULatencyCompensationComponent::ULatencyCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULatencyCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULatencyCompensationComponent::FramePackageSaver(FramePackage& Pack)
{
	Character = Character == nullptr ? Cast<AMainCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Pack.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Pack.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}


	}
}
FramePackage ULatencyCompensationComponent::InterpFrames(const FramePackage& OlderFrame, const FramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpolationFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FramePackage InterpPack;
	InterpPack.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo) 
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInfo& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInfo& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInfo InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpolationFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpolationFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpPack.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);

	}
	return InterpPack;
}

FLagCompensationResult ULatencyCompensationComponent::ConfirmHit(const FramePackage& Pack, AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitChar == nullptr) return FLagCompensationResult();
	FramePackage CurrFrame;
	CacheBoxPos(HitChar, CurrFrame);
	BoxMovement(HitChar, Pack);
	EnableMeshCollision(HitChar, ECollisionEnabled::NoCollision);

	// Enable Collision for the head first 
	UBoxComponent* Head = HitChar->HitCollisionBoxes[FName("head")];
	Head->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Head->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult HitResultConfirm;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			HitResultConfirm,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (HitResultConfirm.bBlockingHit) //the head is hit, return early 
		{
			if (HitResultConfirm.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(HitResultConfirm.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}
			ResetBoxes(HitChar, CurrFrame);
			EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
			return FLagCompensationResult{ true, true };

		}
		else 
		{
		//not hit the head, check the rest body parts 
			for (auto& HitBoxPair : HitChar->HitCollisionBoxes) 
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				HitResultConfirm,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (HitResultConfirm.bBlockingHit)
			{
				if (HitResultConfirm.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(HitResultConfirm.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}
				ResetBoxes(HitChar, CurrFrame);
				EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
				return FLagCompensationResult{ true, false };
			}

		}
	}

	ResetBoxes(HitChar, CurrFrame);
	EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
	return FLagCompensationResult{ false, false };
}

FLagCompensationResult ULatencyCompensationComponent::ProjConfirmHit(const FramePackage& Pack, AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitVelocity, float HitTime)
{
	FramePackage CurrFrame;
	CacheBoxPos(HitChar, CurrFrame);
	BoxMovement(HitChar, Pack);
	EnableMeshCollision(HitChar, ECollisionEnabled::NoCollision);

	// Enable Collision for the head first 
	UBoxComponent* Head = HitChar->HitCollisionBoxes[FName("head")];
	Head->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Head->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxHistoryTime;
	PathParams.LaunchVelocity = InitVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.0f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	//PathParams.DrawDebugTime = 5.f;
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit) //hit the head
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
			}
		}
		ResetBoxes(HitChar, CurrFrame);
		EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
		return FLagCompensationResult{ true, true };

	}
	else //did not hit the head  
	{
		for (auto& HitBoxPair : HitChar->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		} 

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					//DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
			}
			ResetBoxes(HitChar, CurrFrame);
			EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
			return FLagCompensationResult{ true, false };
		}
	}

	ResetBoxes(HitChar, CurrFrame);
	EnableMeshCollision(HitChar, ECollisionEnabled::QueryAndPhysics);
	return FLagCompensationResult{ false, false };
}

void ULatencyCompensationComponent::CacheBoxPos(AMainCharacter* HitChar, FramePackage& OutFramePack)
{
	if (HitChar == nullptr) return;
	for (auto& HitBoxPair : HitChar->HitCollisionBoxes) 
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePack.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}
void ULatencyCompensationComponent::BoxMovement(AMainCharacter* HitChar, const FramePackage& Pack)
{
	if (HitChar == nullptr) return;
	for (auto& HitBoxPair : HitChar->HitCollisionBoxes) 
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Pack.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Pack.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Pack.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	
	}
}

void ULatencyCompensationComponent::ResetBoxes(AMainCharacter* HitChar, const FramePackage& Pack)
{
	if (HitChar == nullptr) return;
	for (auto& HitBoxPair : HitChar->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Pack.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Pack.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Pack.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

	}
}

void ULatencyCompensationComponent::EnableMeshCollision(AMainCharacter* HitChar, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitChar && HitChar->GetMesh())
	{
		HitChar->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULatencyCompensationComponent::FramePackageStory(const FramePackage& Pack, const FColor& Color)
{
	for (auto& BoxInfo : Pack.HitBoxInfo) 
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}
FLagCompensationResult  ULatencyCompensationComponent::LagCompensationAlgorithm(AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	AWall* wall;
	
	FramePackage FrameToCheck = GetFrameToCheck(HitChar, HitTime);
	return ConfirmHit(FrameToCheck, HitChar, TraceStart, HitLocation);
}

FLagCompensationResult ULatencyCompensationComponent::ProjectileLagCompensation(AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitVelocity, float HitTime)
{
	FramePackage FrameToCheck = GetFrameToCheck(HitChar, HitTime);
	return ProjConfirmHit(FrameToCheck, HitChar, TraceStart, InitVelocity, HitTime);
}

FramePackage ULatencyCompensationComponent::GetFrameToCheck(AMainCharacter* HitChar, float HitTime)
{
	bool bReturn =
		HitChar == nullptr ||
		HitChar->GetLatencyCompensation() == nullptr ||
		HitChar->GetLatencyCompensation()->HistoryFrame.GetHead() == nullptr ||
		HitChar->GetLatencyCompensation()->HistoryFrame.GetTail() == nullptr;
	if (bReturn) return FramePackage();
	//frame pack to check if hit is verified
	FramePackage FrameToCheck;

	bool bShoulInterp = true;

	// Frame History of the hit char
	const TDoubleLinkedList<FramePackage>& History = HitChar->GetLatencyCompensation()->HistoryFrame;
	const float OldestTimeHistory = History.GetTail()->GetValue().Time;
	const float NewTimeHistory = History.GetHead()->GetValue().Time;
	if (OldestTimeHistory > HitTime)
	{
		return FramePackage();
	}
	if (OldestTimeHistory == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShoulInterp = false;
	}
	if (NewTimeHistory <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShoulInterp = false;
	}

	TDoubleLinkedList<FramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime)
	{
		//March back until older time < hit time < younger time
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}

	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShoulInterp = false;
	}

	if (bShoulInterp)
	{
		//Interp between frames
		FrameToCheck = InterpFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return FrameToCheck;
}

//
//void ULatencyCompensationComponent::Score_Request_Implementation(AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageSource)
//{
//	FLagCompensationResult Confirm = LagCompensationAlgorithm(HitChar, TraceStart, HitLocation, HitTime);
//	if (Character && HitChar && DamageSource && Confirm.bIsHitConfirmed) 
//	{
//
//	}
//	
//}

void ULatencyCompensationComponent::ProjectileServerScoreRequest_Implementation(AMainCharacter* HitChar, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitVelocity, float HitTime)
{
	FLagCompensationResult Confirm = ProjectileLagCompensation(HitChar, TraceStart, InitVelocity, HitTime);
	if (Character && HitChar && Confirm.bIsHitConfirmed && Character->GetEquippedWeapon())
	{
		UGameplayStatics::ApplyDamage(
			HitChar,
			Character->GetEquippedWeapon()->GetDamage(),
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULatencyCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePack();
}

void ULatencyCompensationComponent::SaveFramePack()
{
	if (Character == nullptr || !Character->HasAuthority()) return;

	if (HistoryFrame.Num() <= 1)
	{
		FramePackage CurrentFrame;
		FramePackageSaver(CurrentFrame);
		HistoryFrame.AddHead(CurrentFrame);
	}
	else
	{
		float HistoryLenght = HistoryFrame.GetHead()->GetValue().Time - HistoryFrame.GetTail()->GetValue().Time;
		while (HistoryLenght > MaxHistoryTime)
		{
			HistoryFrame.RemoveNode(HistoryFrame.GetTail());
			HistoryLenght = HistoryFrame.GetHead()->GetValue().Time - HistoryFrame.GetTail()->GetValue().Time;
		}
		FramePackage CurrentFrame;
		FramePackageSaver(CurrentFrame);
		HistoryFrame.AddHead(CurrentFrame);

		//FramePackageStory(CurrentFrame, FColor::Blue);
	}
}



