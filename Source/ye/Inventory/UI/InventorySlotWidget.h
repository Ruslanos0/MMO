// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "../InventorySlot.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UInventoryComponent;

/**
 * Виджет одного слота инвентаря.
 * Реализует IUserObjectListEntry для использования в ListView/TileView.
 * В Blueprint дочернем классе нужно привязать переменные к элементам UMG.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API UInventorySlotWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    //~ Begin IUserObjectListEntry interface
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    //~ End IUserObjectListEntry interface

    /** Обновить визуал по данным слота */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void RefreshSlot(const FInventorySlot& InSlot);

    /** Получить индекс слота */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    int32 GetSlotIndex() const { return CachedSlot.SlotIndex; }

    /** Пустой ли слот */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    bool IsSlotEmpty() const { return CachedSlot.IsEmpty(); }

    /** Ссылка на компонент инвентаря — public чтобы InventoryWidget мог задать */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
    TWeakObjectPtr<UInventoryComponent> OwnerInventory;

    /** Кешированные данные слота */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
    FInventorySlot CachedSlot;

protected:
    //~ Begin UUserWidget interface
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    //~ End UUserWidget interface

    /** Переопредели в Blueprint чтобы обновить иконку/количество */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    void OnSlotRefreshed(const FInventorySlot& InSlotData);

    /** Правый клик — контекстное меню */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    void OnSlotRightClicked(int32 InSlotIndex);

    /** Иконка предмета */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> ItemIcon;

    /** Текст количества */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> QuantityText;

    /** Рамка (цвет по редкости) */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UBorder> RarityBorder;

private:
    FLinearColor GetRarityColor(EItemRarity Rarity) const;
    void ApplyRarityColor();
};
