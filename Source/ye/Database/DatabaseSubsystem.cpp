// Copyright Epic Games, Inc. All Rights Reserved.

#include "DatabaseSubsystem.h"

DEFINE_LOG_CATEGORY(LogDatabase);

void UDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogDatabase, Log, TEXT("DatabaseSubsystem initialized"));
}

void UDatabaseSubsystem::Deinitialize()
{
    Disconnect();
    Super::Deinitialize();
}

bool UDatabaseSubsystem::Connect(const FString& ConnectionString)
{
    UE_LOG(LogDatabase, Warning, TEXT("Connect called with: %s"), *ConnectionString);

    bool bResult = Connection.Connect(ConnectionString);

    if (bResult)
    {
        UE_LOG(LogDatabase, Warning, TEXT("PostgreSQL connected successfully"));
    }

    return bResult;
}

void UDatabaseSubsystem::Disconnect()
{
    if (Connection.IsConnected())
    {
        Connection.Disconnect();
        UE_LOG(LogDatabase, Log, TEXT("Disconnected from PostgreSQL"));
    }
}

bool UDatabaseSubsystem::IsConnected() const
{
    return Connection.IsConnected();
}
