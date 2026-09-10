// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UOverlay;

/**
 * Главный HUD — показывается во время игры.
 * HP бар, MP бар, уровень, золото, имя персонажа.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API UHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Обновить HP */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetHealth(float Current, float Max);

    /** Обновить MP */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetMana(float Current, float Max);

    /** Обновить XP */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetExperience(float Current, float Max);

    /** Обновить уровень */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetLevel(int32 Level);

    /** Обновить золото */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetGold(int64 Gold);

    /** Обновить имя персонажа */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetCharacterName(const FString& Name);

    /** Показать уведомление на экране */
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowNotification(const FString& Message, float Duration = 3.0f);

protected:
    // ─── UMG Bindings ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ManaBar;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ExperienceBar;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ManaText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> LevelText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> GoldText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CharacterNameText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> NotificationText;

private:
    FTimerHandle NotificationTimer;
};
