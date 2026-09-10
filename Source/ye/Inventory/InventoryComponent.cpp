// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Async/Async.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Инициализируем слоты только на сервере
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        InitSlots();
    }
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Реплицируем Slots только владельцу (клиент видит только свой инвентарь)
    DOREPLIFETIME_CONDITION(UInventoryComponent, Slots, COND_OwnerOnly);
}

void UInventoryComponent::InitSlots()
{
    Slots.SetNum(MaxSlots);
    for (int32 i = 0; i < MaxSlots; ++i)
    {
        Slots[i].SlotIndex = i;
    }
}

// ─── OnRep ──────────────────────────────────────────────────────────────────

void UInventoryComponent::OnRep_Slots()
{
    // Оповещаем UI об изменении на клиенте
    OnInventoryChanged.Broadcast();
}

// ─── Добавление предмета ─────────────────────────────────────────────────────

EInventoryResult UInventoryComponent::AddItem(UItemDefinition* ItemDef, int32 Quantity, int32& OutRemaining)
{
    OutRemaining = 0;

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return EInventoryResult::Failed_NoAuth;
    }

    if (!ItemDef || Quantity <= 0)
    {
        return EInventoryResult::Failed_Invalid;
    }

    int32 LeftToAdd = Quantity;

    // 1. Сначала пробуем добрать в существующие стаки
    if (ItemDef->MaxStackSize > 1)
    {
        while (LeftToAdd > 0)
        {
            int32 StackSlot = FindExistingStack(ItemDef);
            if (StackSlot == INDEX_NONE) break;

            int32 Space = Slots[StackSlot].GetRemainingSpace();
            int32 AddAmount = FMath::Min(LeftToAdd, Space);
            Slots[StackSlot].Quantity += AddAmount;
            LeftToAdd -= AddAmount;
        }
    }

    // 2. Оставшееся кладём в новые пустые слоты
    while (LeftToAdd > 0)
    {
        int32 EmptySlot = FindEmptySlot();
        if (EmptySlot == INDEX_NONE)
        {
            // Инвентарь заполнен — возвращаем остаток
            OutRemaining = LeftToAdd;
            OnInventoryChanged.Broadcast();
            OnItemAdded.Broadcast(ItemDef, Quantity - LeftToAdd);
            return EInventoryResult::Failed_Full;
        }

        int32 AddAmount = FMath::Min(LeftToAdd, ItemDef->MaxStackSize);
        Slots[EmptySlot].ItemDefinition = ItemDef;
        Slots[EmptySlot].Quantity = AddAmount;
        LeftToAdd -= AddAmount;
    }

    OnInventoryChanged.Broadcast();
    OnItemAdded.Broadcast(ItemDef, Quantity);
    return EInventoryResult::Success;
}

// ─── Удаление предмета ───────────────────────────────────────────────────────

EInventoryResult UInventoryComponent::RemoveItem(UItemDefinition* ItemDef, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return EInventoryResult::Failed_NoAuth;
    }

    if (!ItemDef || Quantity <= 0)
    {
        return EInventoryResult::Failed_Invalid;
    }

    if (!HasItem(ItemDef, Quantity))
    {
        return EInventoryResult::Failed_NotFound;
    }

    int32 LeftToRemove = Quantity;

    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty() || Slot.ItemDefinition != ItemDef) continue;

        int32 RemoveAmount = FMath::Min(LeftToRemove, Slot.Quantity);
        Slot.Quantity -= RemoveAmount;
        LeftToRemove -= RemoveAmount;

        if (Slot.Quantity <= 0)
        {
            Slot.Clear();
        }

        if (LeftToRemove <= 0) break;
    }

    OnInventoryChanged.Broadcast();
    OnItemRemoved.Broadcast(ItemDef, Quantity);
    return EInventoryResult::Success;
}

EInventoryResult UInventoryComponent::RemoveItemAtSlot(int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return EInventoryResult::Failed_NoAuth;
    }

    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
    {
        return EInventoryResult::Failed_NotFound;
    }

    FInventorySlot& Slot = Slots[SlotIndex];
    int32 RemoveAmount = (Quantity <= 0) ? Slot.Quantity : FMath::Min(Quantity, Slot.Quantity);

    UItemDefinition* RemovedDef = Slot.ItemDefinition;
    Slot.Quantity -= RemoveAmount;

    if (Slot.Quantity <= 0)
    {
        Slot.Clear();
    }

    OnInventoryChanged.Broadcast();
    OnItemRemoved.Broadcast(RemovedDef, RemoveAmount);
    return EInventoryResult::Success;
}

// ─── Перемещение (Drag & Drop) ───────────────────────────────────────────────

EInventoryResult UInventoryComponent::MoveItem(int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return EInventoryResult::Failed_NoAuth;
    }

    if (!Slots.IsValidIndex(FromSlotIndex) || !Slots.IsValidIndex(ToSlotIndex))
    {
        return EInventoryResult::Failed_Invalid;
    }

    if (FromSlotIndex == ToSlotIndex) return EInventoryResult::Success;

    FInventorySlot& From = Slots[FromSlotIndex];
    FInventorySlot& To   = Slots[ToSlotIndex];

    if (From.IsEmpty()) return EInventoryResult::Failed_NotFound;

    // Если целевой слот занят тем же предметом — объединяем стак
    if (!To.IsEmpty() && To.ItemDefinition == From.ItemDefinition)
    {
        int32 Space = To.GetRemainingSpace();
        int32 Transfer = FMath::Min(Space, From.Quantity);
        To.Quantity += Transfer;
        From.Quantity -= Transfer;

        if (From.Quantity <= 0) From.Clear();
    }
    else
    {
        // Просто меняем местами
        FInventorySlot Temp = From;
        From = To;
        To = Temp;

        // Восстанавливаем корректные индексы
        From.SlotIndex = FromSlotIndex;
        To.SlotIndex   = ToSlotIndex;
    }

    OnInventoryChanged.Broadcast();
    return EInventoryResult::Success;
}

// ─── Выброс предмета ─────────────────────────────────────────────────────────

EInventoryResult UInventoryComponent::DropItem(int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return EInventoryResult::Failed_NoAuth;
    }

    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty())
    {
        return EInventoryResult::Failed_NotFound;
    }

    // TODO: Заспавнить Actor с предметом в мире (WorldItem)
    // Сейчас просто удаляем из инвентаря
    return RemoveItemAtSlot(SlotIndex, Quantity);
}

// ─── Запросы ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::GetSlot(int32 SlotIndex, FInventorySlot& OutSlot) const
{
    if (!Slots.IsValidIndex(SlotIndex)) return false;
    OutSlot = Slots[SlotIndex];
    return true;
}

bool UInventoryComponent::HasItem(UItemDefinition* ItemDef, int32 Quantity) const
{
    return GetItemCount(ItemDef) >= Quantity;
}

int32 UInventoryComponent::GetItemCount(UItemDefinition* ItemDef) const
{
    if (!ItemDef) return 0;

    int32 Total = 0;
    for (const FInventorySlot& Slot : Slots)
    {
        if (!Slot.IsEmpty() && Slot.ItemDefinition == ItemDef)
        {
            Total += Slot.Quantity;
        }
    }
    return Total;
}

float UInventoryComponent::GetCurrentWeight() const
{
    float Total = 0.0f;
    for (const FInventorySlot& Slot : Slots)
    {
        if (!Slot.IsEmpty() && Slot.ItemDefinition)
        {
            Total += Slot.ItemDefinition->Weight * Slot.Quantity;
        }
    }
    return Total;
}

bool UInventoryComponent::IsFull() const
{
    return FindEmptySlot() == INDEX_NONE;
}

int32 UInventoryComponent::GetUsedSlotCount() const
{
    int32 Count = 0;
    for (const FInventorySlot& Slot : Slots)
    {
        if (!Slot.IsEmpty()) ++Count;
    }
    return Count;
}

// ─── Вспомогательные ─────────────────────────────────────────────────────────

int32 UInventoryComponent::FindExistingStack(UItemDefinition* ItemDef) const
{
    for (const FInventorySlot& Slot : Slots)
    {
        if (!Slot.IsEmpty() && Slot.ItemDefinition == ItemDef && Slot.CanAddMore())
        {
            return Slot.SlotIndex;
        }
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindEmptySlot() const
{
    for (const FInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty()) return Slot.SlotIndex;
    }
    return INDEX_NONE;
}

// ─── Загрузка из БД ──────────────────────────────────────────────────────────

void UInventoryComponent::LoadFromDB(const TArray<FInventorySlotData>& DBSlots)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    // Сброс текущих слотов
    InitSlots();

    UAssetManager& AssetManager = UAssetManager::Get();

    for (const FInventorySlotData& DBSlot : DBSlots)
    {
        if (!Slots.IsValidIndex(DBSlot.SlotIndex)) continue;

        // Найти ItemDefinition по ItemKey через Asset Manager
        FPrimaryAssetId AssetID("Item", FName(*DBSlot.ItemKey));
        UItemDefinition* ItemDef = Cast<UItemDefinition>(
            AssetManager.GetPrimaryAssetObject(AssetID)
        );

        if (!ItemDef)
        {
            // Попробуем синхронно загрузить если ещё не в памяти
            FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetID);
            if (!AssetPath.IsNull())
            {
                ItemDef = Cast<UItemDefinition>(AssetPath.TryLoad());
            }
        }

        if (!ItemDef)
        {
            UE_LOG(LogTemp, Warning, TEXT("LoadFromDB: ItemDefinition not found for key '%s'"), *DBSlot.ItemKey);
            continue;
        }

        FInventorySlot& Slot    = Slots[DBSlot.SlotIndex];
        Slot.ItemDefinition     = ItemDef;
        Slot.Quantity           = DBSlot.Quantity;
        Slot.SlotIndex          = DBSlot.SlotIndex;
    }

    UE_LOG(LogTemp, Log, TEXT("LoadFromDB: loaded %d items for character %lld"), DBSlots.Num(), CharacterDBID);
    OnInventoryChanged.Broadcast();
}
