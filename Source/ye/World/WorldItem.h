// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldItem.generated.h"

class UItemDefinition;
class UStaticMeshComponent;
class USphereComponent;
class UInventoryComponent;

/**
 * Предмет лежащий в мире.
 * Когда персонаж подходит — подбирается автоматически или по кнопке взаимодействия.
 */
UCLASS()
class YE_API AWorldItem : public AActor
{
    GENERATED_BODY()

public:
    AWorldItem();

    virtual void BeginPlay() override;

    /** Определение предмета */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TObjectPtr<UItemDefinition> ItemDefinition;

    /** Количество предметов */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 Quantity = 1;

    /** Подбирать автоматически при приближении */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bAutoPickup = true;

    /** Радиус подбора */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float PickupRadius = 100.f;

    /** Попытаться подобрать предмет */
    UFUNCTION(BlueprintCallable, Category = "Item")
    void TryPickup(AActor* Collector);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> PickupSphere;

    UFUNCTION()
    void OnPickupSphereOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    /** Вызывается в Blueprint когда предмет подобран */
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void OnPickedUp(AActor* Collector);
};
