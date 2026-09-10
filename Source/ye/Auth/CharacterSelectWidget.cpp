// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterSelectWidget.h"
#include "CharacterSelectSubsystem.h"
#include "Auth/AuthSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"

// ─── Init ────────────────────────────────────────────────────────────────────

void UCharacterSelectWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Populate class selector
    if (ClassSelector)
    {
        ClassSelector->ClearOptions();
        ClassSelector->AddOption(TEXT("warrior"));
        ClassSelector->AddOption(TEXT("mage"));
        ClassSelector->AddOption(TEXT("rogue"));
        ClassSelector->AddOption(TEXT("paladin"));
        ClassSelector->AddOption(TEXT("hunter"));
        ClassSelector->AddOption(TEXT("priest"));
        ClassSelector->AddOption(TEXT("druid"));
        ClassSelector->AddOption(TEXT("warlock"));
        ClassSelector->SetSelectedIndex(0);
    }

    // Subscribe to subsystem
    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (Sub)
    {
        Sub->OnCharactersLoaded.AddDynamic(this, &UCharacterSelectWidget::HandleCharactersLoaded);
        Sub->OnCharacterCreated.AddDynamic(this, &UCharacterSelectWidget::HandleCharacterCreated);
    }

    // Play button always enabled — PressPlay checks selection internally
    if (PlayButton) PlayButton->SetIsEnabled(true);

    ShowListPanel();

    // Load characters for current account
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UGameInstance* GI = PC->GetGameInstance())
        {
            if (UAuthSubsystem* Auth = GI->GetSubsystem<UAuthSubsystem>())
            {
                if (Auth->IsLoggedIn() && Sub)
                {
                    bIsLoading = true;
                    SetLoading(true);
                    Sub->LoadCharacters(Auth->GetCurrentAccount().ID);
                }
            }
        }
    }
}

void UCharacterSelectWidget::NativeDestruct()
{
    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (Sub)
    {
        Sub->OnCharactersLoaded.RemoveDynamic(this, &UCharacterSelectWidget::HandleCharactersLoaded);
        Sub->OnCharacterCreated.RemoveDynamic(this, &UCharacterSelectWidget::HandleCharacterCreated);
    }

    Super::NativeDestruct();
}

// ─── Actions ─────────────────────────────────────────────────────────────────

void UCharacterSelectWidget::HighlightCharacter(const FCharacterData& Character)
{
    HighlightedCharacter = Character;
    RefreshSelectedCharacterInfo();

    if (PlayButton) PlayButton->SetIsEnabled(Character.IsValid());

    OnCharacterHighlighted(Character);
}

void UCharacterSelectWidget::PressPlay()
{
    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (!Sub) return;

    // Используем выбранного персонажа из субсистемы
    if (!Sub->HasSelectedCharacter())
    {
        ShowMessage(TEXT("Select a character first"), true);
        return;
    }

    OnEnterGame(Sub->GetSelectedCharacter());
}

void UCharacterSelectWidget::SubmitCreateCharacter()
{
    if (bIsLoading) return;

    FString Name  = NewCharacterName ? NewCharacterName->GetText().ToString() : TEXT("");
    FString Class = ClassSelector    ? ClassSelector->GetSelectedOption()     : TEXT("warrior");

    UE_LOG(LogTemp, Log, TEXT("SubmitCreateCharacter: Name='%s' Class='%s' NewCharacterName=%s ClassSelector=%s"),
        *Name, *Class,
        NewCharacterName ? TEXT("Valid") : TEXT("NULL"),
        ClassSelector    ? TEXT("Valid") : TEXT("NULL"));

    if (Name.IsEmpty())
    {
        ShowMessage(TEXT("Character name is required"), true);
        return;
    }

    if (Name.Len() < 3 || Name.Len() > 32)
    {
        ShowMessage(TEXT("Name must be between 3 and 32 characters"), true);
        return;
    }

    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (!Sub) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    UGameInstance* GI = PC->GetGameInstance();
    if (!GI) return;

    UAuthSubsystem* Auth = GI->GetSubsystem<UAuthSubsystem>();
    if (!Auth || !Auth->IsLoggedIn())
    {
        ShowMessage(TEXT("Not logged in"), true);
        return;
    }

    bIsLoading = true;
    SetLoading(true);
    Sub->CreateCharacter(Auth->GetCurrentAccount().ID, Name, Class);
}

void UCharacterSelectWidget::DeleteSelectedCharacter()
{
    if (!HighlightedCharacter.IsValid()) return;

    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (Sub)
    {
        Sub->DeleteCharacter(HighlightedCharacter.ID);
        HighlightedCharacter = FCharacterData{};
        if (PlayButton) PlayButton->SetIsEnabled(false);
        RefreshSelectedCharacterInfo();
    }
}

void UCharacterSelectWidget::ShowCreatePanel()
{
    if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(1);
    if (NewCharacterName) NewCharacterName->SetText(FText::GetEmpty());
}

void UCharacterSelectWidget::ShowListPanel()
{
    if (PanelSwitcher) PanelSwitcher->SetActiveWidgetIndex(0);
}

TArray<FCharacterData> UCharacterSelectWidget::GetCharacters() const
{
    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    return Sub ? Sub->GetCharacters() : TArray<FCharacterData>{};
}

// ─── Delegates ───────────────────────────────────────────────────────────────

void UCharacterSelectWidget::HandleCharactersLoaded(bool bSuccess, const TArray<FCharacterData>& Characters)
{
    bIsLoading = false;
    SetLoading(false);

    if (!bSuccess)
    {
        ShowMessage(TEXT("Failed to load characters"), true);
        return;
    }

    OnCharacterListRefreshed(Characters);

    // Auto-select first character if only one exists
    if (Characters.Num() == 1)
    {
        HighlightCharacter(Characters[0]);
    }
}

void UCharacterSelectWidget::HandleCharacterCreated(bool bSuccess, const FCharacterData& Character)
{
    bIsLoading = false;
    SetLoading(false);

    if (!bSuccess)
    {
        ShowMessage(TEXT("Failed to create character. Name may already be taken."), true);
        return;
    }

    ShowMessage(TEXT("Character created!"), false);
    ShowListPanel();

    UCharacterSelectSubsystem* Sub = GetCharSelectSubsystem();
    if (Sub)
    {
        OnCharacterListRefreshed(Sub->GetCharacters());
        HighlightCharacter(Character);
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void UCharacterSelectWidget::RefreshSelectedCharacterInfo()
{
    if (SelectedCharacterName)
    {
        SelectedCharacterName->SetText(
            HighlightedCharacter.IsValid()
                ? FText::FromString(HighlightedCharacter.Name)
                : FText::FromString(TEXT("No character selected"))
        );
    }

    if (SelectedCharacterLevel)
    {
        SelectedCharacterLevel->SetText(
            HighlightedCharacter.IsValid()
                ? FText::Format(NSLOCTEXT("CS", "Level", "Level {0}"), HighlightedCharacter.Level)
                : FText::GetEmpty()
        );
    }

    if (SelectedCharacterClass)
    {
        SelectedCharacterClass->SetText(
            HighlightedCharacter.IsValid()
                ? FText::FromString(HighlightedCharacter.Class)
                : FText::GetEmpty()
        );
    }
}

UCharacterSelectSubsystem* UCharacterSelectWidget::GetCharSelectSubsystem() const
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UGameInstance* GI = PC->GetGameInstance())
        {
            return GI->GetSubsystem<UCharacterSelectSubsystem>();
        }
    }
    return nullptr;
}
