// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySlot.h"
#include "Database/InventoryDB.h"
#include "InventoryComponent.generated.h"

/** Результат операции с инвентарём */
UENUM(BlueprintType)
enum class EInventoryResult : uint8
{
    Success         UMETA(DisplayName = "Success"),
    Failed_Full     UMETA(DisplayName = "Failed: Inventory Full"),
    Failed_NotFound UMETA(DisplayName = "Failed: Item Not Found"),
    Failed_Invalid  UMETA(DisplayName = "Failed: Invalid Item"),
    Failed_NoAuth   UMETA(DisplayName = "Failed: No Authority"),
};

/** Делегат — вызывается при любом изменении инвентаря (для обновления UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

/** Делегат — вызывается когда предмет добавлен */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, UItemDefinition*, ItemDef, int32, Quantity);

/** Делегат — вызывается когда предмет удалён */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, UItemDefinition*, ItemDef, int32, Quantity);

/**
 * Компонент инвентаря.
 * Добавляется на Character или PlayerState.
 * Вся мутация данных происходит только на сервере (HasAuthority).
 * Слоты реплицируются на клиент автоматически.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class YE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    //~ Begin UActorComponent interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~ End UActorComponent interface

    // ─── Настройки ─────────────────────────────────────────────────────────

    /** Максимальное количество слотов */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 MaxSlots = 30;

    /** Максимальный суммарный вес (0 = без ограничений) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0.0"))
    float MaxWeight = 0.0f;

    // ─── Делегаты ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemRemoved OnItemRemoved;

    // ─── Основные операции (только сервер) ─────────────────────────────────

    /**
     * Добавить предмет в инвентарь.
     * Сначала пытается добавить в существующий стак, затем в свободный слот.
     * @param ItemDef    - определение предмета
     * @param Quantity   - количество
     * @param OutRemaining - сколько не поместилось
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory", meta = (AutoCreateRefTerm = "OutRemaining"))
    EInventoryResult AddItem(UItemDefinition* ItemDef, int32 Quantity, int32& OutRemaining);

    /**
     * Удалить предмет из инвентаря по определению.
     * @param ItemDef  - определение предмета
     * @param Quantity - сколько удалить
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryResult RemoveItem(UItemDefinition* ItemDef, int32 Quantity);

    /**
     * Удалить предмет из конкретного слота.
     * @param SlotIndex - индекс слота
     * @param Quantity  - сколько удалить (0 = весь стак)
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryResult RemoveItemAtSlot(int32 SlotIndex, int32 Quantity = 0);

    /**
     * Переместить предмет из одного слота в другой (drag & drop).
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryResult MoveItem(int32 FromSlotIndex, int32 ToSlotIndex);

    /**
     * Выбросить предмет из слота в мир.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryResult DropItem(int32 SlotIndex, int32 Quantity = 0);

    // ─── Запросы (можно вызывать на клиенте) ────────────────────────────────

    /** Получить все слоты */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FInventorySlot>& GetSlots() const { return Slots; }

    /** Получить конкретный слот */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetSlot(int32 SlotIndex, FInventorySlot& OutSlot) const;

    /** Есть ли предмет в инвентаре */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(UItemDefinition* ItemDef, int32 Quantity = 1) const;

    /** Сколько предметов данного типа в инвентаре */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemCount(UItemDefinition* ItemDef) const;

    /** Текущий суммарный вес */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    float GetCurrentWeight() const;

    /** Заполнен ли инвентарь */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsFull() const;

    /** Количество занятых слотов */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetUsedSlotCount() const;

    /**
     * Загрузить инвентарь из БД данных.
     * Вызывается на сервере после логина персонажа.
     * ItemKey из БД сопоставляется с Data Asset через Asset Manager.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|DB")
    void LoadFromDB(const TArray<FInventorySlotData>& DBSlots);

    /**
     * ID персонажа в БД — нужен для сохранения инвентаря
     */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory|DB")
    int64 CharacterDBID = 0;

private:
    /** Массив слотов — реплицируется на всех клиентов */
    UPROPERTY(ReplicatedUsing = OnRep_Slots)
    TArray<FInventorySlot> Slots;

    /** Вызывается на клиенте после репликации Slots */
    UFUNCTION()
    void OnRep_Slots();

    /** Найти первый слот с этим предметом, у которого есть место для стака */
    int32 FindExistingStack(UItemDefinition* ItemDef) const;

    /** Найти первый пустой слот */
    int32 FindEmptySlot() const;

    /** Инициализировать массив слотов */
    void InitSlots();

    virtual void BeginPlay() override;
};
