// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterSelectSubsystem.h"
#include "Database/DatabaseSubsystem.h"
#include "Database/CharacterDB.h"
#include "Database/PgConnection.h"
#include "Engine/GameInstance.h"
#include "Async/Async.h"

UDatabaseSubsystem* UCharacterSelectSubsystem::GetDB() const
{
    return GetGameInstance()->GetSubsystem<UDatabaseSubsystem>();
}

// ─── LoadCharacters ──────────────────────────────────────────────────────────

void UCharacterSelectSubsystem::LoadCharacters(const FString& AccountID)
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB)
    {
        OnCharactersLoaded.Broadcast(false, {});
        return;
    }

    // Execute on game thread — libpq connection is not thread-safe
    TArray<FCharacterData> Loaded;
    bool bSuccess = UCharacterDB::LoadAccountCharacters(DB, AccountID, Loaded);
    Characters = Loaded;
    OnCharactersLoaded.Broadcast(bSuccess, Characters);
}

// ─── CreateCharacter ─────────────────────────────────────────────────────────

void UCharacterSelectSubsystem::CreateCharacter(
    const FString& AccountID,
    const FString& Name,
    const FString& Class)
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB)
    {
        OnCharacterCreated.Broadcast(false, FCharacterData{});
        return;
    }

    FCharacterData NewChar;
    bool bSuccess = false;

    FString Query = FString::Printf(
        TEXT("INSERT INTO characters (account_id, name, class, pos_x, pos_y, pos_z)"
             " VALUES ('%s', '%s', '%s'::character_class, 211591, 188815, 1120)"
             " RETURNING id, name, class::text, level, experience,"
             " pos_x, pos_y, pos_z, zone_id,"
             " health, health_max, mana, mana_max, gold,"
             " stat_strength, stat_agility, stat_intelligence, stat_stamina, stat_spirit"),
        *AccountID, *Name.Replace(TEXT("'"), TEXT("''")),
        *Class);

    PGresult* R = DB->GetConnection().Exec(Query);
    if (FPgConnection::CheckResult(R, TEXT("CreateCharacter")) && PQntuples(R) > 0)
    {
        NewChar.ID          = FPgConnection::GetInt64(R, 0, 0);
        NewChar.Name        = FPgConnection::GetValue(R, 0, 1);
        NewChar.Class       = FPgConnection::GetValue(R, 0, 2);
        NewChar.Level       = FPgConnection::GetInt  (R, 0, 3);
        NewChar.Experience  = FPgConnection::GetInt64(R, 0, 4);
        NewChar.PosX        = FPgConnection::GetFloat(R, 0, 5);
        NewChar.PosY        = FPgConnection::GetFloat(R, 0, 6);
        NewChar.PosZ        = FPgConnection::GetFloat(R, 0, 7);
        NewChar.ZoneID      = FPgConnection::GetInt  (R, 0, 8);
        NewChar.Health      = FPgConnection::GetInt  (R, 0, 9);
        NewChar.HealthMax   = FPgConnection::GetInt  (R, 0, 10);
        NewChar.Mana        = FPgConnection::GetInt  (R, 0, 11);
        NewChar.ManaMax     = FPgConnection::GetInt  (R, 0, 12);
        NewChar.Gold        = FPgConnection::GetInt64(R, 0, 13);
        NewChar.Strength    = FPgConnection::GetInt  (R, 0, 14);
        NewChar.Agility     = FPgConnection::GetInt  (R, 0, 15);
        NewChar.Intelligence= FPgConnection::GetInt  (R, 0, 16);
        NewChar.Stamina     = FPgConnection::GetInt  (R, 0, 17);
        NewChar.Spirit      = FPgConnection::GetInt  (R, 0, 18);
        bSuccess = true;
    }
    if (R) PQclear(R);

    if (bSuccess) Characters.Add(NewChar);
    OnCharacterCreated.Broadcast(bSuccess, NewChar);
}

// ─── SelectCharacter ─────────────────────────────────────────────────────────

void UCharacterSelectSubsystem::SelectCharacter(const FCharacterData& Character)
{
    SelectedCharacter = Character;
    UE_LOG(LogTemp, Log, TEXT("Selected character: %s (ID=%lld)"), *Character.Name, Character.ID);
    OnCharacterSelected.Broadcast(Character);
}

// ─── DeleteCharacter ─────────────────────────────────────────────────────────

void UCharacterSelectSubsystem::DeleteCharacter(int64 CharacterID)
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB) return;

    FString Query = FString::Printf(
        TEXT("DELETE FROM characters WHERE id=%lld"), CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    FPgConnection::CheckResult(R, TEXT("DeleteCharacter"));
    if (R) PQclear(R);

    Characters.RemoveAll([CharacterID](const FCharacterData& C)
    {
        return C.ID == CharacterID;
    });
}
