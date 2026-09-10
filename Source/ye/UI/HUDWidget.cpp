// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHUDWidget::SetHealth(float Current, float Max)
{
    if (HealthBar)
    {
        float Percent = Max > 0 ? Current / Max : 0.f;
        HealthBar->SetPercent(Percent);

        // Цвет меняется от зелёного к красному
        FLinearColor Color = FLinearColor::LerpUsingHSV(
            FLinearColor(0.8f, 0.1f, 0.1f),  // красный
            FLinearColor(0.1f, 0.7f, 0.1f),  // зелёный
            Percent);
        HealthBar->SetFillColorAndOpacity(Color);
    }

    if (HealthText)
    {
        HealthText->SetText(FText::Format(
            NSLOCTEXT("HUD", "HP", "{0} / {1}"),
            FText::AsNumber(FMath::RoundToInt(Current)),
            FText::AsNumber(FMath::RoundToInt(Max))));
    }
}

void UHUDWidget::SetMana(float Current, float Max)
{
    if (ManaBar)
    {
        ManaBar->SetPercent(Max > 0 ? Current / Max : 0.f);
        ManaBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.3f, 0.9f, 1.f));
    }

    if (ManaText)
    {
        ManaText->SetText(FText::Format(
            NSLOCTEXT("HUD", "MP", "{0} / {1}"),
            FText::AsNumber(FMath::RoundToInt(Current)),
            FText::AsNumber(FMath::RoundToInt(Max))));
    }
}

void UHUDWidget::SetExperience(float Current, float Max)
{
    if (ExperienceBar)
    {
        ExperienceBar->SetPercent(Max > 0 ? Current / Max : 0.f);
        ExperienceBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.7f, 0.1f, 1.f));
    }
}

void UHUDWidget::SetLevel(int32 Level)
{
    if (LevelText)
    {
        LevelText->SetText(FText::Format(
            NSLOCTEXT("HUD", "Level", "Ур. {0}"), Level));
    }
}

void UHUDWidget::SetGold(int64 Gold)
{
    if (GoldText)
    {
        GoldText->SetText(FText::Format(
            NSLOCTEXT("HUD", "Gold", "{0} G"),
            FText::AsNumber(Gold)));
        GoldText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.f)));
    }
}

void UHUDWidget::SetCharacterName(const FString& Name)
{
    if (CharacterNameText)
    {
        CharacterNameText->SetText(FText::FromString(Name));
    }
}

void UHUDWidget::ShowNotification(const FString& Message, float Duration)
{
    if (!NotificationText) return;

    NotificationText->SetText(FText::FromString(Message));
    NotificationText->SetVisibility(ESlateVisibility::HitTestInvisible);

    // Автоскрытие через Duration секунд
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NotificationTimer);
        World->GetTimerManager().SetTimer(NotificationTimer, [this]()
        {
            if (NotificationText)
                NotificationText->SetVisibility(ESlateVisibility::Collapsed);
        }, Duration, false);
    }
}
