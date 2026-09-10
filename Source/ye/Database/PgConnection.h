// Copyright Epic Games, Inc. All Rights Reserved.
// libpq C API wrapper - no std::allocator, no pqxx, no conflicts with UE5

#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include <libpq-fe.h>
THIRD_PARTY_INCLUDES_END

/**
 * RAII wrapper for PGconn using libpq C API.
 * No std::vector, no std::string, no allocator conflicts with UE5.
 */
class YE_API FPgConnection
{
public:
    FPgConnection() : Conn(nullptr) {}

    ~FPgConnection()
    {
        Disconnect();
    }

    // Non-copyable
    FPgConnection(const FPgConnection&) = delete;
    FPgConnection& operator=(const FPgConnection&) = delete;

    bool Connect(const FString& ConnectionString)
    {
        Disconnect();
        Conn = PQconnectdb(TCHAR_TO_UTF8(*ConnectionString));
        if (!Conn || PQstatus(Conn) != CONNECTION_OK)
        {
            UE_LOG(LogTemp, Error, TEXT("PostgreSQL connect failed: %s"),
                Conn ? UTF8_TO_TCHAR(PQerrorMessage(Conn)) : TEXT("null connection"));
            if (Conn) { PQfinish(Conn); Conn = nullptr; }
            return false;
        }
        return true;
    }

    void Disconnect()
    {
        if (Conn)
        {
            PQfinish(Conn);
            Conn = nullptr;
        }
    }

    bool IsConnected() const
    {
        return Conn != nullptr && PQstatus(Conn) == CONNECTION_OK;
    }

    PGconn* Get() const { return Conn; }

    /** Execute a simple query string. Returns nullptr on error. Caller must PQclear result. */
    PGresult* Exec(const FString& Query) const
    {
        if (!IsConnected())
        {
            UE_LOG(LogTemp, Error, TEXT("Exec: not connected"));
            return nullptr;
        }
        PGresult* R = PQexec(Conn, TCHAR_TO_UTF8(*Query));
        if (!R)
        {
            UE_LOG(LogTemp, Error, TEXT("Exec: PQexec returned null: %s"),
                UTF8_TO_TCHAR(PQerrorMessage(Conn)));
        }
        return R;
    }

    /** Check result status and log error. Returns false and calls PQclear on failure. */
    static bool CheckResult(PGresult* Result, const TCHAR* Context)
    {
        if (!Result)
        {
            UE_LOG(LogTemp, Error, TEXT("%s: null result"), Context);
            return false;
        }
        ExecStatusType Status = PQresultStatus(Result);
        if (Status != PGRES_COMMAND_OK && Status != PGRES_TUPLES_OK)
        {
            UE_LOG(LogTemp, Error, TEXT("%s failed [%s]: %s"), Context,
                UTF8_TO_TCHAR(PQresStatus(Status)),
                UTF8_TO_TCHAR(PQresultErrorMessage(Result)));
            PQclear(Result);
            return false;
        }
        return true;
    }

    static FString GetValue(PGresult* R, int Row, int Col)
    {
        if (!R || PQgetisnull(R, Row, Col)) return TEXT("");
        return UTF8_TO_TCHAR(PQgetvalue(R, Row, Col));
    }

    static int32 GetInt(PGresult* R, int Row, int Col)
    {
        return FCString::Atoi(*GetValue(R, Row, Col));
    }

    static int64 GetInt64(PGresult* R, int Row, int Col)
    {
        return FCString::Atoi64(*GetValue(R, Row, Col));
    }

    static float GetFloat(PGresult* R, int Row, int Col)
    {
        return FCString::Atof(*GetValue(R, Row, Col));
    }

    static bool GetBool(PGresult* R, int Row, int Col)
    {
        FString V = GetValue(R, Row, Col);
        return V == TEXT("t") || V == TEXT("true") || V == TEXT("1");
    }

    static int GetRows(PGresult* R)
    {
        return R ? PQntuples(R) : 0;
    }

    static bool IsNull(PGresult* R, int Row, int Col)
    {
        return !R || PQgetisnull(R, Row, Col);
    }

private:
    PGconn* Conn;
};
