// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterPersistenceComponent.h"
#include "InventoryComponent.h"
#include "Database/DatabaseSubsystem.h"
#include "Database/InventoryDB.h"
#include "Database/CharacterDB.h"
#include "Database/PgConnection.h"
#include "ItemDefinition.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Async/Async.h"

UCharacterPersistenceComponent::UCharacterPersistenceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterPersistenceComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCharacterPersistenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Остановить таймер сразу
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimer);
    }

    // Сохранить синхронно на game thread перед выходом — не через AsyncTask
    if (GetOwner() && GetOwner()->HasAuthority() && CharacterData.IsValid())
    {
        UDatabaseSubsystem* DB = GetDB();
        if (DB && DB->IsConnected())
        {
            AActor* Owner = GetOwner();
            if (Owner)
            {
                FVector Pos = Owner->GetActorLocation();
                CharacterData.PosX = Pos.X;
                CharacterData.PosY = Pos.Y;
                CharacterData.PosZ = Pos.Z;
            }
            UCharacterDB::SaveAll(DB, CharacterData);
            UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: saved on EndPlay"));
        }
    }

    Super::EndPlay(EndPlayReason);
}

// ─── Инициализация ───────────────────────────────────────────────────────────

void UCharacterPersistenceComponent::InitFromCharacterData(const FCharacterData& Data)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;

    CharacterData = Data;

    // Передать ID персонажа в InventoryComponent
    if (UInventoryComponent* Inv = GetInventoryComponent())
    {
        Inv->CharacterDBID = Data.ID;
    }

    UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: init for character '%s' (ID=%lld)"),
        *Data.Name, Data.ID);

    // Загрузить инвентарь из БД
    LoadInventoryFromDB();

    // Подписаться на изменения инвентаря для автосохранения
    if (UInventoryComponent* Inv = GetInventoryComponent())
    {
        Inv->OnItemAdded.AddDynamic(this, &UCharacterPersistenceComponent::OnItemAdded);
        Inv->OnItemRemoved.AddDynamic(this, &UCharacterPersistenceComponent::OnItemRemoved);
    }

    // Запустить автосохранение
    StartAutoSave();

    // Установить позицию персонажа из БД с небольшой задержкой
    // чтобы дать движку время полностью заспавнить актора
    if (UWorld* World = GetWorld())
    {
        FVector SpawnPos(Data.PosX, Data.PosY, Data.PosZ);
        FTimerHandle TeleportTimer;
        World->GetTimerManager().SetTimer(TeleportTimer, [this, SpawnPos]()
        {
            if (AActor* Owner = GetOwner())
            {
                Owner->SetActorLocation(SpawnPos, false, nullptr, ETeleportType::TeleportPhysics);
                UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: teleported to (%.0f, %.0f, %.0f)"),
                    SpawnPos.X, SpawnPos.Y, SpawnPos.Z);
            }
        }, 0.1f, false);
    }
}

// ─── Загрузка инвентаря ──────────────────────────────────────────────────────

void UCharacterPersistenceComponent::LoadInventoryFromDB()
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB) return;

    int64 CharID = CharacterData.ID;
    TWeakObjectPtr<UCharacterPersistenceComponent> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, DB, CharID]()
    {
        TArray<FInventorySlotData> DBSlots;
        bool bSuccess = UInventoryDB::LoadInventory(DB, CharID, DBSlots);

        AsyncTask(ENamedThreads::GameThread, [WeakThis, bSuccess, DBSlots]()
        {
            if (!WeakThis.IsValid()) return;

            if (!bSuccess)
            {
                UE_LOG(LogTemp, Error, TEXT("PersistenceComponent: failed to load inventory"));
                return;
            }

            if (UInventoryComponent* Inv = WeakThis->GetInventoryComponent())
            {
                Inv->LoadFromDB(DBSlots);
                UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: inventory loaded (%d slots)"), DBSlots.Num());
            }
        });
    });
}

// ─── Сохранение ──────────────────────────────────────────────────────────────

void UCharacterPersistenceComponent::SaveNow()
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB || !CharacterData.IsValid()) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Обновляем позицию
    FVector Pos = Owner->GetActorLocation();
    CharacterData.PosX = Pos.X;
    CharacterData.PosY = Pos.Y;
    CharacterData.PosZ = Pos.Z;

    FCharacterData DataCopy = CharacterData;
    TWeakObjectPtr<UCharacterPersistenceComponent> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DB, DataCopy]()
    {
        UCharacterDB::SaveAll(DB, DataCopy);
    });

    UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: saving character '%s' at (%.0f, %.0f, %.0f)"),
        *CharacterData.Name, Pos.X, Pos.Y, Pos.Z);
}

// ─── Автосохранение ──────────────────────────────────────────────────────────

void UCharacterPersistenceComponent::StartAutoSave()
{
    if (AutoSaveInterval <= 0.0f) return;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AutoSaveTimer,
            this,
            &UCharacterPersistenceComponent::OnAutoSaveTick,
            AutoSaveInterval,
            true  // looping
        );

        UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: autosave every %.0f seconds"), AutoSaveInterval);
    }
}

void UCharacterPersistenceComponent::OnAutoSaveTick()
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        SaveNow();
    }
}

// ─── Сохранение инвентаря ────────────────────────────────────────────────────

void UCharacterPersistenceComponent::OnItemAdded(UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || CharacterData.ID == 0) return;

    UDatabaseSubsystem* DB = GetDB();
    if (!DB || !DB->IsConnected()) return;

    // Выполняем на game thread — одно соединение не thread-safe
    FString ItemKey = ItemDef->ItemID.ToString();
    FAddItemResult Result = UInventoryDB::AddItem(DB, CharacterData.ID, ItemKey, Quantity);

    if (Result.Success)
    {
        UE_LOG(LogTemp, Log, TEXT("PersistenceComponent: item '%s' x%d saved to DB"), *ItemKey, Quantity);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PersistenceComponent: AddItem DB failed: %s"), *Result.Error);
    }
}

void UCharacterPersistenceComponent::OnItemRemoved(UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || CharacterData.ID == 0) return;

    UDatabaseSubsystem* DB = GetDB();
    if (!DB || !DB->IsConnected()) return;

    FString ItemKey = ItemDef->ItemID.ToString();
    FString Query = FString::Printf(
        TEXT("UPDATE inventory_items SET quantity = quantity - %d"
             " WHERE id = ("
             "  SELECT ii.id FROM inventory_items ii"
             "  JOIN item_templates it ON it.id = ii.item_template_id"
             "  WHERE ii.character_id = %lld AND it.item_key = '%s'"
             "  LIMIT 1);"
             "DELETE FROM inventory_items"
             " WHERE character_id = %lld AND quantity <= 0;"),
        Quantity, CharacterData.ID, *ItemKey, CharacterData.ID);

    PGresult* R = DB->GetConnection().Exec(Query);
    FPgConnection::CheckResult(R, TEXT("OnItemRemoved"));
    if (R) PQclear(R);
}

UDatabaseSubsystem* UCharacterPersistenceComponent::GetDB() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            return GI->GetSubsystem<UDatabaseSubsystem>();
        }
    }
    return nullptr;
}

UInventoryComponent* UCharacterPersistenceComponent::GetInventoryComponent() const
{
    if (AActor* Owner = GetOwner())
    {
        return Owner->FindComponentByClass<UInventoryComponent>();
    }
    return nullptr;
}
