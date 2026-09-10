// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AuthDB.generated.h"

class UDatabaseSubsystem;

/** Result of a login attempt */
UENUM(BlueprintType)
enum class ELoginResult : uint8
{
    Success,
    InvalidCredentials,
    AccountBanned,
    AccountNotVerified,
    DatabaseError,
};

/** Result of a register attempt */
UENUM(BlueprintType)
enum class ERegisterResult : uint8
{
    Success,
    EmailAlreadyExists,
    DatabaseError,
};

/** Account data returned after successful login */
USTRUCT(BlueprintType)
struct YE_API FAccountData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString ID;       // UUID
    UPROPERTY(BlueprintReadOnly) FString Email;
    UPROPERTY(BlueprintReadOnly) FString Role;
    UPROPERTY(BlueprintReadOnly) FString Status;

    bool IsValid() const { return !ID.IsEmpty(); }
};

/** Session data */
USTRUCT(BlueprintType)
struct YE_API FSessionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString SessionID;   // UUID
    UPROPERTY(BlueprintReadOnly) FString AccountID;
    UPROPERTY(BlueprintReadOnly) FString Token;       // raw token (store securely)

    bool IsValid() const { return !SessionID.IsEmpty(); }
};

/**
 * All auth-related DB queries.
 * Passwords are hashed server-side using pgcrypto (crypt + gen_salt).
 */
UCLASS()
class YE_API UAuthDB : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Attempt login. Verifies password using pgcrypto crypt().
     * On success fills OutAccount and creates a session.
     */
    static ELoginResult Login(
        UDatabaseSubsystem* DB,
        const FString&      Email,
        const FString&      Password,
        FAccountData&       OutAccount,
        FSessionData&       OutSession
    );

    /**
     * Register a new account.
     * Password is hashed server-side with bcrypt via pgcrypto.
     */
    static ERegisterResult Register(
        UDatabaseSubsystem* DB,
        const FString&      Email,
        const FString&      Password,
        FAccountData&       OutAccount
    );

    /**
     * Validate an existing session token.
     * Returns false if token is expired or not found.
     */
    static bool ValidateSession(
        UDatabaseSubsystem* DB,
        const FString&      Token,
        FAccountData&       OutAccount
    );

    /**
     * Invalidate (logout) a session by token.
     */
    static bool Logout(
        UDatabaseSubsystem* DB,
        const FString&      Token
    );

    /**
     * Update last_login_at for account.
     */
    static bool UpdateLastLogin(
        UDatabaseSubsystem* DB,
        const FString&      AccountID
    );
};
