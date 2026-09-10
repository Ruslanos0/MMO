// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/DragDropOperation.h"
#include "../InventoryComponent.h"

// ─── IUserObjectListEntry ────────────────────────────────────────────────────

void UInventorySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    // Если используем ListView — ListItemObject должен быть обёрткой UObject над FInventorySlot
    // Для простоты рекомендуется вызывать RefreshSlot напрямую из InventoryWidget
}

// ─── Публичные методы ────────────────────────────────────────────────────────

void UInventorySlotWidget::RefreshSlot(const FInventorySlot& InSlot)
{
    CachedSlot = InSlot;

    // Иконка
    if (ItemIcon)
    {
        if (!InSlot.IsEmpty() && InSlot.ItemDefinition->Icon.IsValid())
        {
            UTexture2D* IconTexture = InSlot.ItemDefinition->Icon.Get();
            if (IconTexture)
            {
                ItemIcon->SetBrushFromTexture(IconTexture);
                ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
            }
        }
        else
        {
            ItemIcon->SetBrushFromTexture(nullptr);
            ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Количество — показываем только если стак > 1
    if (QuantityText)
    {
        if (!InSlot.IsEmpty() && InSlot.Quantity > 1)
        {
            QuantityText->SetText(FText::AsNumber(InSlot.Quantity));
            QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            QuantityText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Рамка редкости
    ApplyRarityColor();

    // Уведомляем Blueprint
    OnSlotRefreshed(InSlot);
}

// ─── Мышь ────────────────────────────────────────────────────────────────────

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        OnSlotRightClicked(CachedSlot.SlotIndex);
        return FReply::Handled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Детектируем начало перетаскивания
        return Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return Reply;
}

// ─── Drag & Drop ─────────────────────────────────────────────────────────────

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (CachedSlot.IsEmpty()) return;

    UDragDropOperation* DragOp = NewObject<UDragDropOperation>(this);
    DragOp->Payload    = this;         // несём ссылку на виджет-источник
    DragOp->DefaultDragVisual = this;  // визуал при перетаскивании

    OutOperation = DragOp;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    if (!InOperation) return false;

    UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
    if (!SourceWidget) return false;

    if (!OwnerInventory.IsValid()) return false;

    // Отправляем RPC на сервер через компонент
    // MoveItem реплицируется через ServerRPC (добавить при необходимости)
    OwnerInventory->MoveItem(SourceWidget->GetSlotIndex(), CachedSlot.SlotIndex);

    return true;
}

// ─── Редкость ────────────────────────────────────────────────────────────────

FLinearColor UInventorySlotWidget::GetRarityColor(EItemRarity Rarity) const
{
    switch (Rarity)
    {
        case EItemRarity::Common:    return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // серый
        case EItemRarity::Uncommon:  return FLinearColor(0.1f, 0.8f, 0.1f, 1.0f); // зелёный
        case EItemRarity::Rare:      return FLinearColor(0.1f, 0.4f, 1.0f, 1.0f); // синий
        case EItemRarity::Epic:      return FLinearColor(0.6f, 0.1f, 0.9f, 1.0f); // фиолетовый
        case EItemRarity::Legendary: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // оранжевый
        default:                     return FLinearColor::White;
    }
}

void UInventorySlotWidget::ApplyRarityColor()
{
    if (!RarityBorder) return;

    if (CachedSlot.IsEmpty())
    {
        RarityBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
    }
    else
    {
        RarityBorder->SetBrushColor(GetRarityColor(CachedSlot.ItemDefinition->Rarity));
    }
}
