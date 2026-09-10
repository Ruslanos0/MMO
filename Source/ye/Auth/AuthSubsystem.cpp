// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthSubsystem.h"
#include "AuthDB.h"
#include "Database/DatabaseSubsystem.h"
#include "Engine/GameInstance.h"
#include "Async/Async.h"

UDatabaseSubsystem* UAuthSubsystem::GetDB() const
{
    return GetGameInstance()->GetSubsystem<UDatabaseSubsystem>();
}

void UAuthSubsystem::Login(const FString& Email, const FString& Password)
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB)
    {
        OnLoginComplete.Broadcast(ELoginResult::DatabaseError, FAccountData{});
        return;
    }

    FString EmailCopy    = Email;
    FString PasswordCopy = Password;
    TWeakObjectPtr<UAuthSubsystem> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, DB, EmailCopy, PasswordCopy]()
    {
        FAccountData Account;
        FSessionData Session;
        ELoginResult Result = UAuthDB::Login(DB, EmailCopy, PasswordCopy, Account, Session);

        AsyncTask(ENamedThreads::GameThread, [WeakThis, Result, Account, Session]()
        {
            if (!WeakThis.IsValid()) return;

            if (Result == ELoginResult::Success)
            {
                WeakThis->CurrentAccount = Account;
                WeakThis->CurrentSession = Session;
            }
            WeakThis->OnLoginComplete.Broadcast(Result, Account);
        });
    });
}

void UAuthSubsystem::Register(const FString& Email, const FString& Password)
{
    UDatabaseSubsystem* DB = GetDB();
    if (!DB)
    {
        OnRegisterComplete.Broadcast(ERegisterResult::DatabaseError, FAccountData{});
        return;
    }

    FString EmailCopy    = Email;
    FString PasswordCopy = Password;
    TWeakObjectPtr<UAuthSubsystem> WeakThis(this);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThis, DB, EmailCopy, PasswordCopy]()
    {
        FAccountData Account;
        ERegisterResult Result = UAuthDB::Register(DB, EmailCopy, PasswordCopy, Account);

        AsyncTask(ENamedThreads::GameThread, [WeakThis, Result, Account]()
        {
            if (!WeakThis.IsValid()) return;

            if (Result == ERegisterResult::Success)
            {
                WeakThis->CurrentAccount = Account;
            }
            WeakThis->OnRegisterComplete.Broadcast(Result, Account);
        });
    });
}

void UAuthSubsystem::Logout()
{
    UDatabaseSubsystem* DB = GetDB();
    FString Token = CurrentSession.Token;

    CurrentAccount = FAccountData{};
    CurrentSession = FSessionData{};

    if (DB && !Token.IsEmpty())
    {
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [DB, Token]()
        {
            UAuthDB::Logout(DB, Token);
        });
    }

    OnLogoutComplete.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: logged out"));
}
