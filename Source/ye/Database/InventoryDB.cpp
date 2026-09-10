// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryDB.h"
#include "DatabaseSubsystem.h"
#include "PgConnection.h"

// ─── LoadInventory ────────────────────────────────────────────────────────────

bool UInventoryDB::LoadInventory(
    UDatabaseSubsystem* DB, int64 CharacterID, TArray<FInventorySlotData>& OutSlots)
{
    if (!DB || !DB->IsConnected()) return false;
    OutSlots.Empty();

    FString Query = FString::Printf(
        TEXT("SELECT instance_id, slot_index, quantity, durability, durability_max,"
             " enchantments::text, item_key, display_name, rarity::text"
             " FROM load_character_inventory(%lld)"), CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!FPgConnection::CheckResult(R, TEXT("LoadInventory"))) return false;

    int Rows = PQntuples(R);
    for (int i = 0; i < Rows; ++i)
    {
        FInventorySlotData Slot;
        Slot.InstanceID   = FPgConnection::GetInt64(R, i, 0);
        Slot.SlotIndex    = FPgConnection::GetInt  (R, i, 1);
        Slot.Quantity     = FPgConnection::GetInt  (R, i, 2);

        if (!PQgetisnull(R, i, 3))
        {
            Slot.Durability    = FPgConnection::GetInt(R, i, 3);
            Slot.DurabilityMax = FPgConnection::GetInt(R, i, 4);
        }

        Slot.Enchantments = FPgConnection::GetValue(R, i, 5);
        Slot.ItemKey      = FPgConnection::GetValue(R, i, 6);
        Slot.DisplayName  = FPgConnection::GetValue(R, i, 7);
        Slot.Rarity       = FPgConnection::GetValue(R, i, 8);

        OutSlots.Add(Slot);
    }

    PQclear(R);
    UE_LOG(LogDatabase, Log, TEXT("LoadInventory: %d slots for char %lld"), OutSlots.Num(), CharacterID);
    return true;
}

// ─── AddItem ──────────────────────────────────────────────────────────────────

FAddItemResult UInventoryDB::AddItem(
    UDatabaseSubsystem* DB, int64 CharacterID, const FString& ItemKey, int32 Quantity)
{
    FAddItemResult Result;
    if (!DB || !DB->IsConnected()) { Result.Error = TEXT("No DB connection"); return Result; }

    FString Query = FString::Printf(
        TEXT("SELECT add_item_to_inventory(%lld::bigint, '%s'::varchar, %d::smallint)"),
        CharacterID, *ItemKey, Quantity);

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!R)
    {
        Result.Error = TEXT("Null result");
        UE_LOG(LogTemp, Error, TEXT("AddItem: null result for query: %s"), *Query);
        return Result;
    }

    ExecStatusType Status = PQresultStatus(R);
    if (Status != PGRES_TUPLES_OK && Status != PGRES_COMMAND_OK)
    {
        Result.Error = UTF8_TO_TCHAR(PQresultErrorMessage(R));
        UE_LOG(LogTemp, Error, TEXT("AddItem query failed: %s"), *Result.Error);
        PQclear(R);
        return Result;
    }

    if (PQntuples(R) > 0)
    {
        FString Json = FPgConnection::GetValue(R, 0, 0);
        UE_LOG(LogTemp, Log, TEXT("AddItem JSON: %s"), *Json);
        Result.Success = Json.Contains(TEXT("\"success\": true")) || Json.Contains(TEXT("\"success\":true"));

        if (!Result.Success)
        {
            int32 S = Json.Find(TEXT("\"error\":\"")) + 9;
            int32 E = Json.Find(TEXT("\""), S);
            Result.Error = Json.Mid(S, E - S);
            if (Result.Error.IsEmpty())
            {
                Result.Error = Json; // показать весь JSON если парсинг не сработал
            }
        }
    }

    PQclear(R);
    return Result;
}

// ─── RemoveItem ───────────────────────────────────────────────────────────────

bool UInventoryDB::RemoveItem(UDatabaseSubsystem* DB, int64 InstanceID, int32 Quantity)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query;
    if (Quantity <= 0)
    {
        Query = FString::Printf(TEXT("DELETE FROM inventory_items WHERE id=%lld"), InstanceID);
    }
    else
    {
        Query = FString::Printf(
            TEXT("UPDATE inventory_items SET quantity=quantity-%d WHERE id=%lld AND quantity>=%d;"
                 "DELETE FROM inventory_items WHERE id=%lld AND quantity<=0"),
            Quantity, InstanceID, Quantity, InstanceID);
    }

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("RemoveItem"));
    if (R) PQclear(R);
    return bOk;
}

// ─── MoveItem ─────────────────────────────────────────────────────────────────

bool UInventoryDB::MoveItem(
    UDatabaseSubsystem* DB, int64 CharacterID, int32 FromSlot, int32 ToSlot)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("WITH moved AS ("
             "  UPDATE inventory_items SET slot_index=-1"
             "  WHERE character_id=%lld AND slot_index=%d RETURNING id"
             "), displaced AS ("
             "  UPDATE inventory_items SET slot_index=%d"
             "  WHERE character_id=%lld AND slot_index=%d"
             ")"
             " UPDATE inventory_items SET slot_index=%d"
             " WHERE id=(SELECT id FROM moved)"),
        CharacterID, FromSlot,
        FromSlot, CharacterID, ToSlot,
        ToSlot);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("MoveItem"));
    if (R) PQclear(R);
    return bOk;
}

// ─── TransferItem ─────────────────────────────────────────────────────────────

bool UInventoryDB::TransferItem(
    UDatabaseSubsystem* DB, int64 FromCharID, int64 ToCharID, int64 InstanceID, int32 Quantity)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("SELECT transfer_item(%lld, %lld, %lld, %d)"),
        FromCharID, ToCharID, InstanceID, Quantity);

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!FPgConnection::CheckResult(R, TEXT("TransferItem"))) return false;

    bool bOk = false;
    if (PQntuples(R) > 0)
    {
        FString Json = FPgConnection::GetValue(R, 0, 0);
        bOk = Json.Contains(TEXT("\"success\": true")) || Json.Contains(TEXT("\"success\":true"));
    }

    PQclear(R);
    return bOk;
}

// ─── UpdateSlotIndex ──────────────────────────────────────────────────────────

bool UInventoryDB::UpdateSlotIndex(UDatabaseSubsystem* DB, int64 InstanceID, int32 NewSlotIndex)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE inventory_items SET slot_index=%d WHERE id=%lld"),
        NewSlotIndex, InstanceID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("UpdateSlotIndex"));
    if (R) PQclear(R);
    return bOk;
}
