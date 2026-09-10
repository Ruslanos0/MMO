// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;
class UImage;
class UItemDefinition;

/**
 * Виджет подсказки предмета — показывается когда персонаж смотрит на WorldItem.
 * Показывает иконку, имя, описание, редкость и подсказку "Нажми E".
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API UItemTooltipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Обновить данные из ItemDefinition */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetItemDefinition(UItemDefinition* ItemDef, int32 Quantity = 1);

    /** Скрыть виджет */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void Hide();

    /** Показать виджет */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void Show();

protected:
    /** Вызывается когда данные обновлены — переопредели в Blueprint */
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnItemSet(UItemDefinition* ItemDef, int32 Quantity);

    // ─── UMG Bindings ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ItemNameText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ItemDescriptionText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ItemRarityText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> QuantityText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> InteractHintText;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> ItemIconImage;
};
