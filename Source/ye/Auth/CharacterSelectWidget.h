// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Database/CharacterDB.h"
#include "CharacterSelectWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
class UComboBoxString;
class UVerticalBox;
class UWidgetSwitcher;
class UCharacterSelectSubsystem;

/**
 * C++ base for WBP_CharacterSelect.
 * Shows list of characters, create new character form, and Play button.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API UCharacterSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ─── Blueprint Events ──────────────────────────────────────────────────

    /** Called when Play is pressed and character is selected — enter the world */
    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect|UI")
    void OnEnterGame(const FCharacterData& Character);

    /** Called when characters list is loaded — refresh the list visually */
    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect|UI")
    void OnCharacterListRefreshed(const TArray<FCharacterData>& Characters);

    /** Called when a character is clicked in the list */
    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect|UI")
    void OnCharacterHighlighted(const FCharacterData& Character);

    /** Show/hide loading overlay */
    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect|UI")
    void SetLoading(bool bLoading);

    /** Show error or status message */
    UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect|UI")
    void ShowMessage(const FString& Message, bool bIsError);

protected:
    //~ Begin UUserWidget interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget interface

    // ─── C++ Actions ──────────────────────────────────────────────────────

    /** Select a character from the list and highlight it */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void HighlightCharacter(const FCharacterData& Character);

    /** Press Play with currently selected character */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void PressPlay();

    /** Submit new character creation form */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void SubmitCreateCharacter();

    /** Delete currently highlighted character */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void DeleteSelectedCharacter();

    /** Switch to create character panel */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void ShowCreatePanel();

    /** Switch back to character list panel */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    void ShowListPanel();

    /** Get currently highlighted character */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    const FCharacterData& GetHighlightedCharacter() const { return HighlightedCharacter; }

    /** Get all loaded characters */
    UFUNCTION(BlueprintCallable, Category = "CharacterSelect|UI")
    TArray<FCharacterData> GetCharacters() const;

    // ─── UMG Bindings ─────────────────────────────────────────────────────

    /** New character name input */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> NewCharacterName;

    /** Class selection dropdown */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> ClassSelector;

    /** Play button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UButton> PlayButton;

    /** Create character button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UButton> CreateButton;

    /** Selected character name display */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SelectedCharacterName;

    /** Selected character level display */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SelectedCharacterLevel;

    /** Selected character class display */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SelectedCharacterClass;

    /** Switcher: index 0 = list, index 1 = create */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> PanelSwitcher;

private:
    UFUNCTION()
    void HandleCharactersLoaded(bool bSuccess, const TArray<FCharacterData>& Characters);

    UFUNCTION()
    void HandleCharacterCreated(bool bSuccess, const FCharacterData& Character);

    void RefreshSelectedCharacterInfo();

    UCharacterSelectSubsystem* GetCharSelectSubsystem() const;

    FCharacterData HighlightedCharacter;
    bool bIsLoading = false;
};
