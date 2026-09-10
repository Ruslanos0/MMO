// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoginWidget.h"
#include "AuthSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"

// ─── Init ────────────────────────────────────────────────────────────────────

void ULoginWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UAuthSubsystem* Auth = GetAuthSubsystem();
    if (Auth)
    {
        Auth->OnLoginComplete.AddDynamic(this, &ULoginWidget::HandleLoginComplete);
        Auth->OnRegisterComplete.AddDynamic(this, &ULoginWidget::HandleRegisterComplete);
    }

    // Start on login tab
    SwitchToLoginTab();
}

void ULoginWidget::NativeDestruct()
{
    UAuthSubsystem* Auth = GetAuthSubsystem();
    if (Auth)
    {
        Auth->OnLoginComplete.RemoveDynamic(this, &ULoginWidget::HandleLoginComplete);
        Auth->OnRegisterComplete.RemoveDynamic(this, &ULoginWidget::HandleRegisterComplete);
    }

    Super::NativeDestruct();
}

// ─── Actions ─────────────────────────────────────────────────────────────────

void ULoginWidget::SubmitLogin()
{
    if (bIsLoading) return;

    FString Email    = LoginEmail    ? LoginEmail->GetText().ToString()    : TEXT("");
    FString Password = LoginPassword ? LoginPassword->GetText().ToString() : TEXT("");

    if (Email.IsEmpty() || Password.IsEmpty())
    {
        SetStatusText(TEXT("Email and password are required"));
        return;
    }

    if (!Email.Contains(TEXT("@")))
    {
        SetStatusText(TEXT("Invalid email format"));
        return;
    }

    bIsLoading = true;
    SetLoading(true);
    SetStatusText(TEXT(""), false);

    UAuthSubsystem* Auth = GetAuthSubsystem();
    if (Auth)
    {
        Auth->Login(Email, Password);
    }
}

void ULoginWidget::SubmitRegister()
{
    if (bIsLoading) return;

    FString Email    = RegisterEmail           ? RegisterEmail->GetText().ToString()           : TEXT("");
    FString Password = RegisterPassword        ? RegisterPassword->GetText().ToString()        : TEXT("");
    FString Confirm  = RegisterPasswordConfirm ? RegisterPasswordConfirm->GetText().ToString() : TEXT("");

    if (Email.IsEmpty() || Password.IsEmpty())
    {
        SetStatusText(TEXT("Email and password are required"));
        return;
    }

    if (!Email.Contains(TEXT("@")))
    {
        SetStatusText(TEXT("Invalid email format"));
        return;
    }

    if (Password.Len() < 6)
    {
        SetStatusText(TEXT("Password must be at least 6 characters"));
        return;
    }

    if (Password != Confirm)
    {
        SetStatusText(TEXT("Passwords do not match"));
        return;
    }

    bIsLoading = true;
    SetLoading(true);
    SetStatusText(TEXT(""), false);

    UAuthSubsystem* Auth = GetAuthSubsystem();
    if (Auth)
    {
        Auth->Register(Email, Password);
    }
}

void ULoginWidget::SwitchToLoginTab()
{
    if (TabSwitcher) TabSwitcher->SetActiveWidgetIndex(0);
    SetStatusText(TEXT(""), false);
}

void ULoginWidget::SwitchToRegisterTab()
{
    if (TabSwitcher) TabSwitcher->SetActiveWidgetIndex(1);
    SetStatusText(TEXT(""), false);
}

// ─── Delegates ───────────────────────────────────────────────────────────────

void ULoginWidget::HandleLoginComplete(ELoginResult Result, const FAccountData& Account)
{
    bIsLoading = false;
    SetLoading(false);

    switch (Result)
    {
    case ELoginResult::Success:
        SetStatusText(TEXT(""), false);
        OnLoginSuccess();
        break;

    case ELoginResult::InvalidCredentials:
        SetStatusText(TEXT("Invalid email or password"));
        OnAuthError(TEXT("Invalid email or password"));
        break;

    case ELoginResult::AccountBanned:
        SetStatusText(TEXT("This account has been banned"));
        OnAuthError(TEXT("Account banned"));
        break;

    case ELoginResult::AccountNotVerified:
        SetStatusText(TEXT("Please verify your email"));
        OnAuthError(TEXT("Email not verified"));
        break;

    default:
        SetStatusText(TEXT("Connection error. Please try again"));
        OnAuthError(TEXT("Database error"));
        break;
    }
}

void ULoginWidget::HandleRegisterComplete(ERegisterResult Result, const FAccountData& Account)
{
    bIsLoading = false;
    SetLoading(false);

    switch (Result)
    {
    case ERegisterResult::Success:
        SetStatusText(TEXT("Account created! You can now log in"), false);
        SwitchToLoginTab();
        OnRegisterSuccess();
        break;

    case ERegisterResult::EmailAlreadyExists:
        SetStatusText(TEXT("This email is already registered"));
        OnAuthError(TEXT("Email already exists"));
        break;

    default:
        SetStatusText(TEXT("Connection error. Please try again"));
        OnAuthError(TEXT("Database error"));
        break;
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

void ULoginWidget::SetStatusText(const FString& Message, bool bIsError)
{
    if (!StatusText) return;

    StatusText->SetText(FText::FromString(Message));
    StatusText->SetColorAndOpacity(
        bIsError
            ? FSlateColor(FLinearColor(1.f, 0.3f, 0.3f, 1.f))  // red
            : FSlateColor(FLinearColor(0.3f, 1.f, 0.3f, 1.f))  // green
    );
    StatusText->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

UAuthSubsystem* ULoginWidget::GetAuthSubsystem() const
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            return GI->GetSubsystem<UAuthSubsystem>();
        }
    }
    // Fallback via PlayerController
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UGameInstance* GI = PC->GetGameInstance())
        {
            return GI->GetSubsystem<UAuthSubsystem>();
        }
    }
    return nullptr;
}
