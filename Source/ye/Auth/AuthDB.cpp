// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthDB.h"
#include "Database/DatabaseSubsystem.h"
#include "Database/PgConnection.h"

// ─── Login ───────────────────────────────────────────────────────────────────

ELoginResult UAuthDB::Login(
    UDatabaseSubsystem* DB,
    const FString& Email, const FString& Password,
    FAccountData& OutAccount, FSessionData& OutSession)
{
    if (!DB || !DB->IsConnected()) return ELoginResult::DatabaseError;

    // Find account and verify password with pgcrypto
    FString Query = FString::Printf(
        TEXT("SELECT id::text, email, role::text, status::text,"
             " (password_hash = crypt('%s', password_hash)) AS valid"
             " FROM accounts WHERE email = '%s'"),
        *Password.Replace(TEXT("'"), TEXT("''")),
        *Email.Replace(TEXT("'"), TEXT("''")));

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!FPgConnection::CheckResult(R, TEXT("Login query")))
        return ELoginResult::DatabaseError;

    if (PQntuples(R) == 0)
    {
        PQclear(R);
        return ELoginResult::InvalidCredentials;
    }

    FString Status = FPgConnection::GetValue(R, 0, 3);
    if (Status == TEXT("banned"))
    {
        PQclear(R);
        return ELoginResult::AccountBanned;
    }

    bool bValid = FPgConnection::GetBool(R, 0, 4);
    if (!bValid)
    {
        PQclear(R);
        return ELoginResult::InvalidCredentials;
    }

    OutAccount.ID     = FPgConnection::GetValue(R, 0, 0);
    OutAccount.Email  = FPgConnection::GetValue(R, 0, 1);
    OutAccount.Role   = FPgConnection::GetValue(R, 0, 2);
    OutAccount.Status = Status;
    PQclear(R);

    // Create session
    FString SessionQuery = FString::Printf(
        TEXT("INSERT INTO sessions (account_id, token_hash, expires_at)"
             " VALUES ('%s', encode(gen_random_bytes(32),'hex'), NOW() + INTERVAL '7 days')"
             " RETURNING id::text, token_hash"),
        *OutAccount.ID);

    PGresult* SR = DB->GetConnection().Exec(SessionQuery);
    if (FPgConnection::CheckResult(SR, TEXT("Create session")) && PQntuples(SR) > 0)
    {
        OutSession.SessionID = FPgConnection::GetValue(SR, 0, 0);
        OutSession.Token     = FPgConnection::GetValue(SR, 0, 1);
        OutSession.AccountID = OutAccount.ID;
    }
    if (SR) PQclear(SR);

    // Update last login
    UpdateLastLogin(DB, OutAccount.ID);

    UE_LOG(LogTemp, Log, TEXT("Login success: %s"), *Email);
    return ELoginResult::Success;
}

// ─── Register ────────────────────────────────────────────────────────────────

ERegisterResult UAuthDB::Register(
    UDatabaseSubsystem* DB,
    const FString& Email, const FString& Password,
    FAccountData& OutAccount)
{
    if (!DB || !DB->IsConnected()) return ERegisterResult::DatabaseError;

    // Check if email exists
    FString CheckQuery = FString::Printf(
        TEXT("SELECT id FROM accounts WHERE email='%s'"),
        *Email.Replace(TEXT("'"), TEXT("''")));

    PGresult* CR = DB->GetConnection().Exec(CheckQuery);
    if (!FPgConnection::CheckResult(CR, TEXT("Register check")))
        return ERegisterResult::DatabaseError;

    if (PQntuples(CR) > 0)
    {
        PQclear(CR);
        return ERegisterResult::EmailAlreadyExists;
    }
    PQclear(CR);

    // Insert with bcrypt hash
    FString InsertQuery = FString::Printf(
        TEXT("INSERT INTO accounts (email, password_hash, status)"
             " VALUES ('%s', crypt('%s', gen_salt('bf', 10)), 'active')"
             " RETURNING id::text, email, role::text, status::text"),
        *Email.Replace(TEXT("'"), TEXT("''")),
        *Password.Replace(TEXT("'"), TEXT("''")));

    PGresult* R = DB->GetConnection().Exec(InsertQuery);
    if (!FPgConnection::CheckResult(R, TEXT("Register insert")))
        return ERegisterResult::DatabaseError;

    if (PQntuples(R) > 0)
    {
        OutAccount.ID     = FPgConnection::GetValue(R, 0, 0);
        OutAccount.Email  = FPgConnection::GetValue(R, 0, 1);
        OutAccount.Role   = FPgConnection::GetValue(R, 0, 2);
        OutAccount.Status = FPgConnection::GetValue(R, 0, 3);
    }
    PQclear(R);

    UE_LOG(LogTemp, Log, TEXT("Register success: %s"), *Email);
    return ERegisterResult::Success;
}

// ─── ValidateSession ─────────────────────────────────────────────────────────

bool UAuthDB::ValidateSession(
    UDatabaseSubsystem* DB, const FString& Token, FAccountData& OutAccount)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("SELECT a.id::text, a.email, a.role::text, a.status::text"
             " FROM sessions s JOIN accounts a ON a.id=s.account_id"
             " WHERE s.token_hash='%s' AND s.expires_at>NOW()"),
        *Token.Replace(TEXT("'"), TEXT("''")));

    PGresult* R = DB->GetConnection().Exec(Query);
    if (!FPgConnection::CheckResult(R, TEXT("ValidateSession"))) return false;

    if (PQntuples(R) == 0) { PQclear(R); return false; }

    OutAccount.ID     = FPgConnection::GetValue(R, 0, 0);
    OutAccount.Email  = FPgConnection::GetValue(R, 0, 1);
    OutAccount.Role   = FPgConnection::GetValue(R, 0, 2);
    OutAccount.Status = FPgConnection::GetValue(R, 0, 3);
    PQclear(R);
    return true;
}

// ─── Logout ──────────────────────────────────────────────────────────────────

bool UAuthDB::Logout(UDatabaseSubsystem* DB, const FString& Token)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("DELETE FROM sessions WHERE token_hash='%s'"),
        *Token.Replace(TEXT("'"), TEXT("''")));

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("Logout"));
    if (R) PQclear(R);
    return bOk;
}

// ─── UpdateLastLogin ─────────────────────────────────────────────────────────

bool UAuthDB::UpdateLastLogin(UDatabaseSubsystem* DB, const FString& AccountID)
{
    if (!DB || !DB->IsConnected()) return false;

    FString Query = FString::Printf(
        TEXT("UPDATE accounts SET last_login_at=NOW() WHERE id='%s'"),
        *AccountID);

    PGresult* R = DB->GetConnection().Exec(Query);
    bool bOk = FPgConnection::CheckResult(R, TEXT("UpdateLastLogin"));
    if (R) PQclear(R);
    return bOk;
}
