// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "InventorySlot.generated.h"

/**
 * Один слот инвентаря.
 * Хранит ссылку на определение предмета и текущее количество.
 * Реплицируется как часть массива в InventoryComponent.
 */
USTRUCT(BlueprintType)
struct YE_API FInventorySlot
{
    GENERATED_BODY()

    FInventorySlot()
        : ItemDefinition(nullptr)
        , Quantity(0)
        , SlotIndex(INDEX_NONE)
    {}

    FInventorySlot(UItemDefinition* InDefinition, int32 InQuantity, int32 InSlotIndex)
        : ItemDefinition(InDefinition)
        , Quantity(InQuantity)
        , SlotIndex(InSlotIndex)
    {}

    /** Определение предмета (статические данные) */
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UItemDefinition> ItemDefinition;

    /** Текущее количество предметов в слоте */
    UPROPERTY(BlueprintReadOnly)
    int32 Quantity;

    /** Индекс слота в массиве инвентаря */
    UPROPERTY(BlueprintReadOnly)
    int32 SlotIndex;

    /** Пустой ли слот */
    bool IsEmpty() const { return ItemDefinition == nullptr || Quantity <= 0; }

    /** Можно ли добавить ещё предметы в этот слот */
    bool CanAddMore() const
    {
        if (!ItemDefinition) return false;
        return Quantity < ItemDefinition->MaxStackSize;
    }

    /** Сколько ещё можно добавить */
    int32 GetRemainingSpace() const
    {
        if (!ItemDefinition) return 0;
        return ItemDefinition->MaxStackSize - Quantity;
    }

    /** Сброс слота */
    void Clear()
    {
        ItemDefinition = nullptr;
        Quantity = 0;
    }

    bool operator==(const FInventorySlot& Other) const
    {
        return SlotIndex == Other.SlotIndex;
    }
};
