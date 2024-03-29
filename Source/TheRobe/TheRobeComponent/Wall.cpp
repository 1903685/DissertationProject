// Fill out your copyright notice in the Description page of Project Settings.


#include "Wall.h"
#include "Components/BoxComponent.h"
#include "TheRobe/Character/MainCharacter.h"
#include "TheRobe/Weapon/Weapon.h"



// Sets default values
AWall::AWall()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(RootComponent);
	SetRootComponent(WallMesh);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	AreaBox->SetupAttachment(RootComponent);
	AreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWall::BeginPlay()
{
	Super::BeginPlay();
	
	//the server will be in charge of all wall objects
	if (HasAuthority())
	{
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaBox->OnComponentBeginOverlap.AddDynamic(this, &AWall::OnOverlapBegin);
		AreaBox->OnComponentEndOverlap.AddDynamic(this, &AWall::OnEndOverlap);

	}
}

void AWall::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
	if (MainChar)
	{
		insideTheBox = 1;
		MainChar->NetUpdateFrequency = 90.f;
		MainChar->MinNetUpdateFrequency = 45.f;
	}
	UE_LOG(LogTemp, Log, TEXT("Bool value is: %d"), insideTheBox);
}

void AWall::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
	if (MainChar)
	{
		insideTheBox = 0;
		MainChar->NetUpdateFrequency = 66.f;
		MainChar->MinNetUpdateFrequency = 33.f;
	}

	UE_LOG(LogTemp, Log, TEXT("Bool value is: %d"), insideTheBox);
}


void AWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

