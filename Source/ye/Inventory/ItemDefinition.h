// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

/**
 * Тип предмета — определяет поведение и слот экипировки
 */
UENUM(BlueprintType)
enum class EItemType : uint8
{
    None        UMETA(DisplayName = "None"),
    Weapon      UMETA(DisplayName = "Weapon"),
    Armor       UMETA(DisplayName = "Armor"),
    Helmet      UMETA(DisplayName = "Helmet"),
    Boots       UMETA(DisplayName = "Boots"),
    Gloves      UMETA(DisplayName = "Gloves"),
    Ring        UMETA(DisplayName = "Ring"),
    Necklace    UMETA(DisplayName = "Necklace"),
    Consumable  UMETA(DisplayName = "Consumable"),
    Material    UMETA(DisplayName = "Material"),
    Quest       UMETA(DisplayName = "Quest"),
    Misc        UMETA(DisplayName = "Misc"),
};

/**
 * Редкость предмета
 */
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
};

/**
 * Описание предмета — создаётся как Data Asset в редакторе.
 * Не хранит состояние, только статические данные.
 */
UCLASS(BlueprintType)
class YE_API UItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** Уникальный ID предмета (используется для сетевой синхронизации и сохранений) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemID;

    /** Отображаемое имя */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    /** Описание для tooltip */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;

    /** Тип предмета */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType = EItemType::Misc;

    /** Редкость */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemRarity Rarity = EItemRarity::Common;

    /** Иконка для UI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TSoftObjectPtr<UTexture2D> Icon;

    /** Максимальный размер стака (1 = нестакаемый) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxStackSize = 1;

    /** Вес одного предмета */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
    float Weight = 0.1f;

    /** Цена продажи торговцу */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy")
    int32 SellPrice = 0;

    /** Можно ли выбросить */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bCanDrop = true;

    /** Можно ли передать другому игроку */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bCanTrade = true;

    // UPrimaryDataAsset interface
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
