// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Database/CharacterDB.h"
#include "CharacterSelectSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharactersLoaded, bool, bSuccess, const TArray<FCharacterData>&, Characters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterCreated, bool, bSuccess, const FCharacterData&, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, const FCharacterData&, Character);

/**
 * Manages character list and selection between login and gameplay.
 */
UCLASS()
class YE_API UCharacterSelectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ─── Events ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "CharacterSelect")
    FOnCharactersLoaded OnCharactersLoaded;

    UPROPERTY(BlueprintAssignable, Category = "CharacterSelect")
    FOnCharacterCreated OnCharacterCreated;

    UPROPERTY(BlueprintAssignable, Category = "CharacterSelect")
    FOnCharacterSelected OnCharacterSelected;

    // ─── Actions ───────────────────────────────────────────────────────────

    /**
     * Load all characters for an account from DB.
     * Broadcasts OnCharactersLoaded when done.
     */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    void LoadCharacters(const FString& AccountID);

    /**
     * Create a new character for an account.
     */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    void CreateCharacter(const FString& AccountID, const FString& Name, const FString& Class);

    /**
     * Select a character to play — stores it and broadcasts event.
     */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    void SelectCharacter(const FCharacterData& Character);

    /**
     * Delete a character by ID.
     */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    void DeleteCharacter(int64 CharacterID);

    // ─── State ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    const TArray<FCharacterData>& GetCharacters() const { return Characters; }

    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    bool HasSelectedCharacter() const { return SelectedCharacter.IsValid(); }

    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    const FCharacterData& GetSelectedCharacter() const { return SelectedCharacter; }

    /** Get character name by index */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    FString GetCharacterName(int32 Index) const
    {
        return Characters.IsValidIndex(Index) ? Characters[Index].Name : TEXT("");
    }

    /** Get character level by index */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    int32 GetCharacterLevel(int32 Index) const
    {
        return Characters.IsValidIndex(Index) ? Characters[Index].Level : 0;
    }

    /** Get character class by index */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    FString GetCharacterClass(int32 Index) const
    {
        return Characters.IsValidIndex(Index) ? Characters[Index].Class : TEXT("");
    }

    /** Select character by index */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
    void SelectCharacterByIndex(int32 Index)
    {
        if (Characters.IsValidIndex(Index))
        {
            SelectCharacter(Characters[Index]);
        }
    }

private:
    TArray<FCharacterData> Characters;
    FCharacterData         SelectedCharacter;

    UDatabaseSubsystem* GetDB() const;
};
