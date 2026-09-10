// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Inventory/ItemDefinition.h"

void UItemTooltipWidget::SetItemDefinition(UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef) return;

    if (ItemNameText)
    {
        ItemNameText->SetText(ItemDef->DisplayName);

        // Цвет по редкости
        FLinearColor RarityColor = FLinearColor::White;
        switch (ItemDef->Rarity)
        {
        case EItemRarity::Common:    RarityColor = FLinearColor(0.7f, 0.7f, 0.7f); break;
        case EItemRarity::Uncommon:  RarityColor = FLinearColor(0.1f, 0.8f, 0.1f); break;
        case EItemRarity::Rare:      RarityColor = FLinearColor(0.1f, 0.4f, 1.0f); break;
        case EItemRarity::Epic:      RarityColor = FLinearColor(0.6f, 0.1f, 0.9f); break;
        case EItemRarity::Legendary: RarityColor = FLinearColor(1.0f, 0.5f, 0.0f); break;
        }
        ItemNameText->SetColorAndOpacity(FSlateColor(RarityColor));
    }

    if (ItemDescriptionText)
    {
        ItemDescriptionText->SetText(ItemDef->Description);
    }

    if (ItemRarityText)
    {
        // Конвертируем enum в строку
        const UEnum* EnumPtr = StaticEnum<EItemRarity>();
        FString RarityStr = EnumPtr
            ? EnumPtr->GetDisplayNameTextByValue((int64)ItemDef->Rarity).ToString()
            : TEXT("");
        ItemRarityText->SetText(FText::FromString(RarityStr));
    }

    if (QuantityText)
    {
        if (Quantity > 1)
        {
            QuantityText->SetText(FText::Format(
                NSLOCTEXT("Tooltip", "Qty", "x{0}"), Quantity));
            QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            QuantityText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (InteractHintText)
    {
        InteractHintText->SetText(
            NSLOCTEXT("Tooltip", "PickupHint", "[E] Подобрать"));
    }

    if (ItemIconImage && ItemDef->Icon.IsValid())
    {
        UTexture2D* Tex = ItemDef->Icon.Get();
        if (Tex) ItemIconImage->SetBrushFromTexture(Tex);
    }

    OnItemSet(ItemDef, Quantity);
    Show();
}

void UItemTooltipWidget::Hide()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UItemTooltipWidget::Show()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
