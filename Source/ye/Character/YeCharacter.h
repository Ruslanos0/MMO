// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "YeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInventoryComponent;
class UCharacterPersistenceComponent;

/**
 * Базовый класс игрового персонажа.
 * Камера от третьего лица, движение через Enhanced Input.
 */
UCLASS()
class YE_API AYeCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AYeCharacter();

    //~ Begin AActor interface
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    //~ End AActor interface

    // ─── Компоненты ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
    TObjectPtr<UCharacterPersistenceComponent> PersistenceComponent;

    // ─── Настройки камеры ──────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraDistance = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float CameraHeight = 150.f;

    // ─── Настройки движения ────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float SprintSpeed = 700.f;

protected:
    // ─── Input ─────────────────────────────────────────────────────────────

    void Move(const struct FInputActionValue& Value);
    void Look(const struct FInputActionValue& Value);
    void StartSprint();
    void StopSprint();
    void ToggleInventory();
    void Interact();

    // ─── Input Actions ─────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> InventoryAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> InteractAction;

    // ─── Tooltip ───────────────────────────────────────────────────────────

    /** Класс виджета подсказки — задать в Blueprint */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UItemTooltipWidget> TooltipWidgetClass;

    /** Класс HUD виджета — задать в Blueprint */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UHUDWidget> HUDWidgetClass;

    /** Получить HUD виджет */
    UFUNCTION(BlueprintCallable, Category = "UI")
    class UHUDWidget* GetHUDWidget() const { return HUDWidget; }

private:
    bool bIsInventoryOpen = false;

    UPROPERTY()
    TObjectPtr<class UItemTooltipWidget> TooltipWidget;

    UPROPERTY()
    TObjectPtr<class UHUDWidget> HUDWidget;

    /** Последний предмет на который смотрит игрок */
    UPROPERTY()
    TWeakObjectPtr<class AWorldItem> LookedAtItem;

    void UpdateItemTooltip();
};
