// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterDB.generated.h"

class UDatabaseSubsystem;

/**
 * Character data loaded from DB
 */
USTRUCT(BlueprintType)
struct YE_API FCharacterData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int64   ID              = 0;
    UPROPERTY(BlueprintReadOnly) FString Name;
    UPROPERTY(BlueprintReadOnly) FString Class;
    UPROPERTY(BlueprintReadOnly) int32   Level           = 1;
    UPROPERTY(BlueprintReadOnly) int64   Experience      = 0;

    // Position
    UPROPERTY(BlueprintReadOnly) float   PosX            = 0.f;
    UPROPERTY(BlueprintReadOnly) float   PosY            = 0.f;
    UPROPERTY(BlueprintReadOnly) float   PosZ            = 0.f;
    UPROPERTY(BlueprintReadOnly) int32   ZoneID          = 1;

    // Resources
    UPROPERTY(BlueprintReadOnly) int32   Health          = 100;
    UPROPERTY(BlueprintReadOnly) int32   HealthMax       = 100;
    UPROPERTY(BlueprintReadOnly) int32   Mana            = 100;
    UPROPERTY(BlueprintReadOnly) int32   ManaMax         = 100;

    // Currency
    UPROPERTY(BlueprintReadOnly) int64   Gold            = 0;

    // Stats
    UPROPERTY(BlueprintReadOnly) int32   Strength        = 10;
    UPROPERTY(BlueprintReadOnly) int32   Agility         = 10;
    UPROPERTY(BlueprintReadOnly) int32   Intelligence    = 10;
    UPROPERTY(BlueprintReadOnly) int32   Stamina         = 10;
    UPROPERTY(BlueprintReadOnly) int32   Spirit          = 10;

    bool IsValid() const { return ID > 0; }
};

/**
 * Static helper — all character-related DB queries.
 * Call only on server (HasAuthority).
 */
UCLASS()
class YE_API UCharacterDB : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Load character by ID.
     */
    static bool LoadCharacter(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        FCharacterData&     OutData
    );

    /**
     * Load all characters for an account.
     */
    static bool LoadAccountCharacters(
        UDatabaseSubsystem*     DB,
        const FString&          AccountID,
        TArray<FCharacterData>& OutCharacters
    );

    /**
     * Save character position (called frequently during gameplay).
     * Calls save_character_position() procedure.
     */
    static bool SavePosition(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        float               X,
        float               Y,
        float               Z,
        int32               ZoneID
    );

    /**
     * Save current health and mana.
     */
    static bool SaveResources(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        int32               Health,
        int32               Mana
    );

    /**
     * Save experience and level.
     */
    static bool SaveProgress(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        int32               Level,
        int64               Experience
    );

    /**
     * Save gold amount.
     */
    static bool SaveGold(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        int64               Gold
    );

    /**
     * Set character online/offline status.
     */
    static bool SetOnlineStatus(
        UDatabaseSubsystem* DB,
        int64               CharacterID,
        bool                bIsOnline
    );

    /**
     * Full character save — position, resources, progress, gold.
     * Use on logout or periodic autosave.
     */
    static bool SaveAll(
        UDatabaseSubsystem*     DB,
        const FCharacterData&   Data
    );
};
