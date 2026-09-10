// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldItem.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

AWorldItem::AWorldItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Меш предмета — видимый для трассировки
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    SetRootComponent(Mesh);

    // Сфера подбора
    PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
    PickupSphere->SetupAttachment(Mesh);
    PickupSphere->SetSphereRadius(PickupRadius);
    PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AWorldItem::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoPickup)
    {
        PickupSphere->OnComponentBeginOverlap.AddDynamic(
            this, &AWorldItem::OnPickupSphereOverlap);
    }
}

void AWorldItem::OnPickupSphereOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    TryPickup(OtherActor);
}

void AWorldItem::TryPickup(AActor* Collector)
{
    if (!Collector || !ItemDefinition) return;

    // Только на сервере
    if (!HasAuthority()) return;

    // Найти InventoryComponent у Collector
    UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();
    if (!Inventory) return;

    int32 Remaining = 0;
    EInventoryResult Result = Inventory->AddItem(ItemDefinition, Quantity, Remaining);

    if (Result == EInventoryResult::Success || Remaining < Quantity)
    {
        UE_LOG(LogTemp, Log, TEXT("WorldItem: '%s' picked up by '%s'"),
            *ItemDefinition->DisplayName.ToString(),
            *Collector->GetName());

        OnPickedUp(Collector);

        // Уничтожить предмет в мире
        Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldItem: inventory full for '%s'"),
            *Collector->GetName());
    }
}
