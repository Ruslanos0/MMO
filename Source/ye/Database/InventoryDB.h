// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Inventory/InventorySlot.h"
#include "InventoryDB.generated.h"

class UDatabaseSubsystem;

/**
 * Data loaded from DB for one inventory slot.
 * Matches load_character_inventory() procedure output.
 */
USTRUCT(BlueprintType)
struct YE_API FInventorySlotData
{
    GENERATED_BODY()

    /** inventory_items.id — unique instance ID in DB */
    UPROPERTY(BlueprintReadOnly) int64  InstanceID    = 0;
    UPROPERTY(BlueprintReadOnly) int32  SlotIndex     = 0;
    UPROPERTY(BlueprintReadOnly) int32  Quantity      = 0;
    UPROPERTY(BlueprintReadOnly) int32  Durability    = -1;
    UPROPERTY(BlueprintReadOnly) int32  DurabilityMax = -1;
    UPROPERTY(BlueprintReadOnly) FString ItemKey;       // matches ItemID in UE5 Data Asset
    UPROPERTY(BlueprintReadOnly) FString DisplayName;
    UPROPERTY(BlueprintReadOnly) FString Rarity;
    UPROPERTY(BlueprintReadOnly) FString Enchantments; // raw JSON string
};

/**
 * Result of add_item_to_inventory() call
 */
USTRUCT(BlueprintType)
struct YE_API FAddItemResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) bool   Success   = false;
    UPROPERTY(BlueprintReadOnly) FString Error;
    UPROPERTY(BlueprintReadOnly) int32  Remaining = 0; // items that didn't fit
};

/**
 * Static helper — all inventory-related DB queries.
 * Call only on server (HasAuthority).
 */
UCLASS()
class YE_API UInventoryDB : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Load all inventory slots for a character.
     * Calls load_character_inventory(p_character_id).
     */
    static bool LoadInventory(
        UDatabaseSubsystem*         DB,
        int64                       CharacterID,
        TArray<FInventorySlotData>& OutSlots
    );

    /**
     * Add item to character inventory with auto-stacking.
     * Calls add_item_to_inventory(character_id, item_key, quantity).
     */
    static FAddItemResult AddItem(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        const FString&      ItemKey,
        int32               Quantity = 1
    );

    /**
     * Remove a quantity from a specific inventory instance.
     * Quantity=0 means remove entire stack.
     */
    static bool RemoveItem(
        UDatabaseSubsystem* DB,
        int64               InstanceID,
        int32               Quantity = 0
    );

    /**
     * Move item between slots (drag & drop).
     */
    static bool MoveItem(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        int32               FromSlot,
        int32               ToSlot
    );

    /**
     * Transfer item to another character.
     * Calls transfer_item(from, to, item_id, quantity).
     */
    static bool TransferItem(
        UDatabaseSubsystem* DB,
        int64               FromCharID,
        int64               ToCharID,
        int64               InstanceID,
        int32               Quantity = 1
    );

    /**
     * Save slot index after drag & drop (update only slot_index).
     */
    static bool UpdateSlotIndex(
        UDatabaseSubsystem* DB,
        int64               InstanceID,
        int32               NewSlotIndex
    );
};
