// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterDB.h"
#include "DatabaseSubsystem.h"
#include "PgConnection.h"

// ─── Helper ──────────────────────────────────────────────────────────────────

static void FillCharacterData(PGresult* R, int Row, FCharacterData& D)
{
    D.ID          = FPgConnection::GetInt64(R, Row, 0);
    D.Name        = FPgConnection::GetValue(R, Row, 1);
    D.Class       = FPgConnection::GetValue(R, Row, 2);
    D.Level       = FPgConnection::GetInt  (R, Row, 3);
    D.Experience  = FPgConnection::GetInt64(R, Row, 4);
    D.PosX        = FPgConnection::GetFloat(R, Row, 5);
    D.PosY        = FPgConnection::GetFloat(R, Row, 6);
    D.PosZ        = FPgConnection::GetFloat(R, Row, 7);
    D.ZoneID      = FPgConnection::GetInt  (R, Row, 8);
    D.Health      = FPgConnection::GetInt  (R, Row, 9);
    D.HealthMax   = FPgConnection::GetInt  (R, Row, 10);
    D.Mana        = FPgConnection::GetInt  (R, Row, 11);
    D.ManaMax     = FPgConnection::GetInt  (R, Row, 12);
    D.Gold        = FPgConnection::GetInt64(R, Row, 13);
    D.Strength    = FPgConnection::GetInt  (R, Row, 14);
    D.Agility     = FPgConnection::GetInt  (R, Row, 15);
    D.Intelligence= FPgConnection::GetInt  (R, Row, 16);
    D.Stamina     = FPgConnection::GetInt  (R, Row, 17);
    D.Spirit      = FPgConnection::GetInt  (R, Row, 18);
}

static const TCHAR* CharSelectSQL =
    TEXT("SELECT id, name, class::text, level, experience,"
         " pos_x, pos_y, pos_z, zone_id,"
         " health, health_max, mana, mana_max, gold,"
         " stat_strength, stat_agility, stat_intelligence, stat_stamina, stat_spirit"
         " FROM characters");

// ─── LoadCharacter ────────────────────────────────────────────────────────────

bool UCharacterDB::LoadCharacter(
    UDatabaseSubsystem* DB, int64 CharacterID, FCharacterData& OutData)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("%s WHERE id = %lld"), CharSelectSQL, CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!FPgConnection::CheckResult(R, TEXT("LoadCharacter"))) return false;

    if (PQntuples(R) == 0) { PQclear(R); return false; }

    FillCharacterData(R, 0, OutData);
    PQclear(R);

    UE_LOG(LogDatabase, Log, TEXT("LoadCharacter: '%s' (ID=%lld)"), *OutData.Name, CharacterID);
    return true;
}

// ─── LoadAccountCharacters ────────────────────────────────────────────────────

bool UCharacterDB::LoadAccountCharacters(
    UDatabaseSubsystem* DB, const FString& AccountID, TArray<FCharacterData>& OutCharacters)
{
    if (!DB || !DB->IsConnected()) 
    {
        UE_LOG(LogTemp, Error, TEXT("LoadAccountCharacters: no DB connection"));
        return false;
    }
    OutCharacters.Empty();

    UE_LOG(LogTemp, Log, TEXT("LoadAccountCharacters: loading for account '%s'"), *AccountID);

    FString Query = FString::Printf(
        TEXT("%s WHERE account_id = '%s' ORDER BY id"), CharSelectSQL, *AccountID);

    UE_LOG(LogTemp, Log, TEXT("LoadAccountCharacters query: %s"), *Query);

    PGresult* R = DB->GetConnection().Exec(Query);

    if (!R)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadAccountCharacters: null result"));
        return false;
    }

    ExecStatusType Status = PQresultStatus(R);
    UE_LOG(LogTemp, Log, TEXT("LoadAccountCharacters status: %s, rows: %d"),
        UTF8_TO_TCHAR(PQresStatus(Status)), PQntuples(R));

    if (!FPgConnection::CheckResult(R, TEXT("LoadAccountCharacters"))) return false;

    int Rows = PQntuples(R);
    for (int i = 0; i < Rows; ++i)
    {
        FCharacterData D;
        FillCharacterData(R, i, D);
        OutCharacters.Add(D);
    }

    PQclear(R);
    return true;
}

// ─── SavePosition ─────────────────────────────────────────────────────────────

bool UCharacterDB::SavePosition(
    UDatabaseSubsystem* DB, int64 CharacterID, float X, float Y, float Z, int32 ZoneID)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("SELECT save_character_position(%lld, %f, %f, %f, %d)"),
        CharacterID, X, Y, Z, ZoneID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SavePosition"));
    if (R) PQclear(R);
    return bOk;
}

// ─── SaveResources ────────────────────────────────────────────────────────────

bool UCharacterDB::SaveResources(
    UDatabaseSubsystem* DB, int64 CharacterID, int32 Health, int32 Mana)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE characters SET health=%d, mana=%d, updated_at=NOW() WHERE id=%lld"),
        Health, Mana, CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SaveResources"));
    if (R) PQclear(R);
    return bOk;
}

// ─── SaveProgress ─────────────────────────────────────────────────────────────

bool UCharacterDB::SaveProgress(
    UDatabaseSubsystem* DB, int64 CharacterID, int32 Level, int64 Experience)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE characters SET level=%d, experience=%lld, updated_at=NOW() WHERE id=%lld"),
        Level, Experience, CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SaveProgress"));
    if (R) PQclear(R);
    return bOk;
}

// ─── SaveGold ─────────────────────────────────────────────────────────────────

bool UCharacterDB::SaveGold(UDatabaseSubsystem* DB, int64 CharacterID, int64 Gold)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE characters SET gold=%lld, updated_at=NOW() WHERE id=%lld"),
        Gold, CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SaveGold"));
    if (R) PQclear(R);
    return bOk;
}

// ─── SetOnlineStatus ──────────────────────────────────────────────────────────

bool UCharacterDB::SetOnlineStatus(UDatabaseSubsystem* DB, int64 CharacterID, bool bIsOnline)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE characters SET is_online=%s, last_seen_at=NOW(), updated_at=NOW() WHERE id=%lld"),
        bIsOnline ? TEXT("true") : TEXT("false"), CharacterID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SetOnlineStatus"));
    if (R) PQclear(R);
    return bOk;
}

// ─── SaveAll ──────────────────────────────────────────────────────────────────

bool UCharacterDB::SaveAll(UDatabaseSubsystem* DB, const FCharacterData& Data)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE characters SET"
             " level=%d, experience=%lld,"
             " pos_x=%f, pos_y=%f, pos_z=%f, zone_id=%d,"
             " health=%d, mana=%d, gold=%lld,"
             " is_online=false, last_seen_at=NOW(), updated_at=NOW()"
             " WHERE id=%lld"),
        Data.Level, Data.Experience,
        Data.PosX, Data.PosY, Data.PosZ, Data.ZoneID,
        Data.Health, Data.Mana, Data.Gold,
        Data.ID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("SaveAll"));
    if (R) PQclear(R);

    if (bOk)
    {
        UE_LOG(LogDatabase, Log, TEXT("SaveAll: '%s' (ID=%lld)"), *Data.Name, Data.ID);
    }

    return bOk;
}
