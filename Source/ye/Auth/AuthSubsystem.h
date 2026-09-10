// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Auth/AuthDB.h"
#include "AuthSubsystem.generated.h"

class UDatabaseSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginComplete, ELoginResult, Result, const FAccountData&, Account);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRegisterComplete, ERegisterResult, Result, const FAccountData&, Account);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogoutComplete);

/**
 * GameInstance Subsystem — manages auth state for the current session.
 * Stores the logged-in account and session token.
 * All heavy DB work runs on a background thread.
 */
UCLASS()
class YE_API UAuthSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ─── Events ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Auth")
    FOnLoginComplete OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "Auth")
    FOnRegisterComplete OnRegisterComplete;

    UPROPERTY(BlueprintAssignable, Category = "Auth")
    FOnLogoutComplete OnLogoutComplete;

    // ─── Actions ───────────────────────────────────────────────────────────

    /**
     * Attempt login with email and password.
     * Result is broadcast via OnLoginComplete on the game thread.
     */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    void Login(const FString& Email, const FString& Password);

    /**
     * Register a new account.
     * Result is broadcast via OnRegisterComplete on the game thread.
     */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    void Register(const FString& Email, const FString& Password);

    /**
     * Logout current account. Invalidates session in DB.
     */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    void Logout();

    // ─── State ─────────────────────────────────────────────────────────────

    /** Is a user currently logged in */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    bool IsLoggedIn() const { return CurrentAccount.IsValid(); }

    /** Get current account data */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    const FAccountData& GetCurrentAccount() const { return CurrentAccount; }

    /** Get current account ID as string */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    FString GetCurrentAccountID() const { return CurrentAccount.ID; }

    /** Get current session token */
    UFUNCTION(BlueprintCallable, Category = "Auth")
    const FString& GetSessionToken() const { return CurrentSession.Token; }

    /** Get current session */
    const FSessionData& GetCurrentSession() const { return CurrentSession; }

private:
    FAccountData CurrentAccount;
    FSessionData CurrentSession;

    UDatabaseSubsystem* GetDB() const;
};
