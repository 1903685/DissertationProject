#include "LatencyCompensationComponent.h"
#include "TheRobe/Character/MainCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

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
void ULatencyCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

		FramePackageStory(CurrentFrame, FColor::Blue);
	}

}



