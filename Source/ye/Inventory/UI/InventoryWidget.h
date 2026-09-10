// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../InventorySlot.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class UTextBlock;
class UWrapBox;

/**
 * Главный виджет инвентаря.
 * Управляет сеткой слотов и слушает делегаты InventoryComponent.
 * Наследуй в Blueprint, расставь WrapBox/UniformGridPanel и назови переменные.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Инициализировать виджет компонентом инвентаря.
     * Вызывай после AddToViewport.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void InitInventory(UInventoryComponent* InInventoryComponent);

    /** Принудительно перерисовать все слоты */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void RefreshInventory();

    /** Получить InventoryComponent */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    UInventoryComponent* GetInventoryComponent() const { return InventoryComponent.Get(); }

protected:
    //~ Begin UUserWidget interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget interface

    /**
     * Вызывается когда нужно создать виджет для конкретного слота.
     * Переопредели в Blueprint, верни созданный UInventorySlotWidget.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    UInventorySlotWidget* CreateSlotWidget();

    /**
     * Вызывается после полного обновления списка слотов.
     * Используй чтобы обновить вес, счётчик слотов и т.д.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|UI")
    void OnInventoryRefreshed();

    /** Класс виджета слота — задаётся в Blueprint через дочерний класс */
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|UI")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    /** Контейнер для слотов (WrapBox или UniformGridPanel в Blueprint) */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UWrapBox> SlotsContainer;

    /** Текст текущего веса */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> WeightText;

    /** Текст занятых слотов */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SlotCountText;

    /** Ссылка на компонент */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|UI")
    TWeakObjectPtr<UInventoryComponent> InventoryComponent;

    /** Все созданные виджеты слотов */
    UPROPERTY()
    TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

private:
    /** Подписаться на делегаты компонента */
    void BindToInventory();

    /** Отписаться от делегатов компонента */
    void UnbindFromInventory();

    UFUNCTION()
    void OnInventoryChanged();

    /** Создать виджеты слотов при первой инициализации */
    void BuildSlotWidgets();

    /** Обновить текстовые подсказки (вес, кол-во слотов) */
    void UpdateStats();
};
