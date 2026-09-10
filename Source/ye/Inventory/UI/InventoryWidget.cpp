// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryWidget.h"
#include "InventorySlotWidget.h"
#include "../InventoryComponent.h"
#include "Components/WrapBox.h"
#include "Components/TextBlock.h"

// ─── Жизненный цикл ──────────────────────────────────────────────────────────

void UInventoryWidget::NativeDestruct()
{
    UnbindFromInventory();
    Super::NativeDestruct();
}

// ─── Инициализация ───────────────────────────────────────────────────────────

void UInventoryWidget::InitInventory(UInventoryComponent* InInventoryComponent)
{
    if (!InInventoryComponent) return;

    UnbindFromInventory();
    InventoryComponent = InInventoryComponent;

    BuildSlotWidgets();
    BindToInventory();
    RefreshInventory();
}

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Вызывается когда виджет добавлен на экран
    // Если к этому моменту компонент уже задан — строим слоты
    if (InventoryComponent.IsValid() && SlotWidgets.Num() == 0)
    {
        BuildSlotWidgets();
        RefreshInventory();
    }
}

void UInventoryWidget::BuildSlotWidgets()
{
    if (!InventoryComponent.IsValid() || !SlotWidgetClass) return;

    // Очищаем старые виджеты
    SlotWidgets.Empty();
    if (SlotsContainer)
    {
        SlotsContainer->ClearChildren();
    }

    const int32 SlotCount = InventoryComponent->MaxSlots;
    SlotWidgets.Reserve(SlotCount);

    for (int32 i = 0; i < SlotCount; ++i)
    {
        UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
        if (!SlotWidget) continue;

        // Передаём ссылку на компонент каждому слоту
        SlotWidget->OwnerInventory = InventoryComponent;

        SlotWidgets.Add(SlotWidget);

        if (SlotsContainer)
        {
            SlotsContainer->AddChildToWrapBox(SlotWidget);
        }
    }
}

// ─── Обновление ──────────────────────────────────────────────────────────────

void UInventoryWidget::RefreshInventory()
{
    if (!InventoryComponent.IsValid()) return;

    const TArray<FInventorySlot>& Slots = InventoryComponent->GetSlots();

    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (!SlotWidgets[i]) continue;

        if (Slots.IsValidIndex(i))
        {
            SlotWidgets[i]->RefreshSlot(Slots[i]);
        }
    }

    UpdateStats();
    OnInventoryRefreshed();
}

void UInventoryWidget::UpdateStats()
{
    if (!InventoryComponent.IsValid()) return;

    if (WeightText)
    {
        if (InventoryComponent->MaxWeight > 0.0f)
        {
            FText WeightFormat = FText::Format(
                NSLOCTEXT("Inventory", "WeightFormat", "{0} / {1} kg"),
                FText::AsNumber(FMath::RoundToInt(InventoryComponent->GetCurrentWeight())),
                FText::AsNumber(FMath::RoundToInt(InventoryComponent->MaxWeight))
            );
            WeightText->SetText(WeightFormat);
        }
        else
        {
            FText WeightFormat = FText::Format(
                NSLOCTEXT("Inventory", "WeightFormatNoMax", "{0} kg"),
                FText::AsNumber(FMath::RoundToInt(InventoryComponent->GetCurrentWeight()))
            );
            WeightText->SetText(WeightFormat);
        }
    }

    if (SlotCountText)
    {
        FText SlotFormat = FText::Format(
            NSLOCTEXT("Inventory", "SlotFormat", "{0} / {1}"),
            FText::AsNumber(InventoryComponent->GetUsedSlotCount()),
            FText::AsNumber(InventoryComponent->MaxSlots)
        );
        SlotCountText->SetText(SlotFormat);
    }
}

// ─── Делегаты ────────────────────────────────────────────────────────────────

void UInventoryWidget::BindToInventory()
{
    if (!InventoryComponent.IsValid()) return;
    InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::OnInventoryChanged);
}

void UInventoryWidget::UnbindFromInventory()
{
    if (!InventoryComponent.IsValid()) return;
    InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryChanged);
}

void UInventoryWidget::OnInventoryChanged()
{
    RefreshInventory();
}