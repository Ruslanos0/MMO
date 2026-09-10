// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Database/PgConnection.h"
#include "DatabaseSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDatabase, Log, All);

UENUM(BlueprintType)
enum class EDBResult : uint8
{
    Success,
    Failed_Connection,
    Failed_Query,
    Failed_NotFound,
};

/**
 * GameInstance Subsystem — manages PostgreSQL connection via libpq (C API).
 * libpq has no std::allocator conflicts with UE5 memory system.
 */
UCLASS()
class YE_API UDatabaseSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Database")
    bool Connect(const FString& ConnectionString);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void Disconnect();

    UFUNCTION(BlueprintCallable, Category = "Database")
    bool IsConnected() const;

    FPgConnection& GetConnection() { return Connection; }

private:
    FPgConnection Connection;
};
