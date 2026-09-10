// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Auth/AuthDB.h"
#include "LoginWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class UWidgetSwitcher;
class UAuthSubsystem;

/**
 * C++ base for WBP_Login.
 * Handles Login and Register tabs.
 * Bind UMG elements by name in Blueprint child class.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class YE_API ULoginWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ─── Blueprint Events ──────────────────────────────────────────────────

    /** Called when login succeeds — switch to character select */
    UFUNCTION(BlueprintImplementableEvent, Category = "Auth|UI")
    void OnLoginSuccess();

    /** Called when register succeeds */
    UFUNCTION(BlueprintImplementableEvent, Category = "Auth|UI")
    void OnRegisterSuccess();

    /** Called on any auth error — show message to user */
    UFUNCTION(BlueprintImplementableEvent, Category = "Auth|UI")
    void OnAuthError(const FString& Message);

    /** Show loading spinner */
    UFUNCTION(BlueprintImplementableEvent, Category = "Auth|UI")
    void SetLoading(bool bLoading);

protected:
    //~ Begin UUserWidget interface
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget interface

    // ─── C++ Actions (call from Blueprint buttons) ─────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Auth|UI")
    void SubmitLogin();

    UFUNCTION(BlueprintCallable, Category = "Auth|UI")
    void SubmitRegister();

    UFUNCTION(BlueprintCallable, Category = "Auth|UI")
    void SwitchToLoginTab();

    UFUNCTION(BlueprintCallable, Category = "Auth|UI")
    void SwitchToRegisterTab();

    // ─── UMG Bindings (name must match exactly in Blueprint) ───────────────

    /** Email field on Login tab */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> LoginEmail;

    /** Password field on Login tab */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> LoginPassword;

    /** Email field on Register tab */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> RegisterEmail;

    /** Password field on Register tab */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> RegisterPassword;

    /** Confirm password field on Register tab */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> RegisterPasswordConfirm;

    /** Error/status message text */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> StatusText;

    /** Switcher between Login and Register tabs (index 0 = login, 1 = register) */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> TabSwitcher;

private:
    UFUNCTION()
    void HandleLoginComplete(ELoginResult Result, const FAccountData& Account);

    UFUNCTION()
    void HandleRegisterComplete(ERegisterResult Result, const FAccountData& Account);

    void SetStatusText(const FString& Message, bool bIsError = true);

    UAuthSubsystem* GetAuthSubsystem() const;

    bool bIsLoading = false;
};
