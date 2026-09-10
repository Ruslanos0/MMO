// Copyright Epic Games, Inc. All Rights Reserved.

#include "YeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/CharacterPersistenceComponent.h"
#include "Auth/CharacterSelectSubsystem.h"
#include "World/WorldItem.h"
#include "UI/ItemTooltipWidget.h"
#include "UI/HUDWidget.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"

AYeCharacter::AYeCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── Spring Arm ────────────────────────────────────────────────────────
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength        = CameraDistance;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag        = true;
    SpringArm->CameraLagSpeed          = 10.f;
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, CameraHeight));

    // ─── Camera ───────────────────────────────────────────────────────────
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // ─── Movement ─────────────────────────────────────────────────────────
    GetCharacterMovement()->bOrientRotationToMovement   = true;
    GetCharacterMovement()->RotationRate                = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed                = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity               = 500.f;
    GetCharacterMovement()->AirControl                  = 0.35f;
    bUseControllerRotationYaw = false;

    // ─── Inventory ────────────────────────────────────────────────────────
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

    // ─── Persistence ──────────────────────────────────────────────────────
    PersistenceComponent = CreateDefaultSubobject<UCharacterPersistenceComponent>(TEXT("Persistence"));
}

void AYeCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Добавить Input Mapping Context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Инициализировать данные персонажа из субсистемы
    if (HasAuthority())
    {
        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GI = World->GetGameInstance())
            {
                if (UCharacterSelectSubsystem* CharSel = GI->GetSubsystem<UCharacterSelectSubsystem>())
                {
                    if (CharSel->HasSelectedCharacter())
                    {
                        const FCharacterData& Data = CharSel->GetSelectedCharacter();
                        PersistenceComponent->InitFromCharacterData(Data);
                    }
                }
            }
        }
    }

    // Создать HUD
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (HUDWidgetClass && PC->IsLocalPlayerController())
        {
            HUDWidget = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport(1);

                if (UWorld* World = GetWorld())
                {
                    if (UGameInstance* GI = World->GetGameInstance())
                    {
                        if (UCharacterSelectSubsystem* CharSel = GI->GetSubsystem<UCharacterSelectSubsystem>())
                        {
                            if (CharSel->HasSelectedCharacter())
                            {
                                const FCharacterData& Data = CharSel->GetSelectedCharacter();
                                HUDWidget->SetCharacterName(Data.Name);
                                HUDWidget->SetLevel(Data.Level);
                                HUDWidget->SetHealth(Data.Health, Data.HealthMax);
                                HUDWidget->SetMana(Data.Mana, Data.ManaMax);
                                HUDWidget->SetGold(Data.Gold);
                                HUDWidget->SetExperience(Data.Experience, Data.Level * 1000.f);
                            }
                        }
                    }
                }
            }
        }
    }
}

void AYeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent called"));

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AYeCharacter::Move);
        }
        if (LookAction)
        {
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AYeCharacter::Look);
        }
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started,  this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Started,   this, &AYeCharacter::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed,  this, &AYeCharacter::StopSprint);
        }
        if (InventoryAction)
        {
            EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AYeCharacter::ToggleInventory);
        }
        if (InteractAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("Binding InteractAction"));
            EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AYeCharacter::Interact);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InteractAction is NULL!"));
        }
    }
}

// ─── Movement ────────────────────────────────────────────────────────────────

void AYeCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller)
    {
        const FRotator Rotation    = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDir, MovementVector.Y);
        AddMovementInput(RightDir,   MovementVector.X);
    }
}

void AYeCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();

    if (Controller)
    {
        AddControllerYawInput(LookVector.X);
        AddControllerPitchInput(LookVector.Y);
    }
}

void AYeCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AYeCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ─── Инвентарь ───────────────────────────────────────────────────────────────

void AYeCharacter::ToggleInventory()
{
    // Делегируем в Blueprint через событие
    // Blueprint переопределит это через BlueprintImplementableEvent если нужно
    // Пока используем прямое управление виджетом через PlayerController
}

void AYeCharacter::Interact()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FVector CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);

    FVector End = CameraLoc + CameraRot.Vector() * 1000.f;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // Используем SweepSingle с радиусом 30 чтобы легче попасть в предмет
    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, CameraLoc, End, FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(30.f),
        Params);

    UE_LOG(LogTemp, Warning, TEXT("Interact: bHit=%s Actor=%s"),
        bHit ? TEXT("true") : TEXT("false"),
        bHit && Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"));

    if (bHit && Hit.GetActor())
    {
        if (AWorldItem* Item = Cast<AWorldItem>(Hit.GetActor()))
        {
            Item->TryPickup(this);
        }
    }
}

// ─── Tooltip ─────────────────────────────────────────────────────────────────

void AYeCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateItemTooltip();
}

void AYeCharacter::UpdateItemTooltip()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FVector CameraLoc;
    FRotator CameraRot;
    PC->GetPlayerViewPoint(CameraLoc, CameraRot);

    FVector End = CameraLoc + CameraRot.Vector() * 1000.f;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit, CameraLoc, End, FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(30.f),
        Params);

    AWorldItem* NewItem = bHit ? Cast<AWorldItem>(Hit.GetActor()) : nullptr;

    // Создать виджет если ещё нет
    if (!TooltipWidget && TooltipWidgetClass && PC->IsLocalPlayerController())
    {
        TooltipWidget = CreateWidget<UItemTooltipWidget>(PC, TooltipWidgetClass);
        if (TooltipWidget)
        {
            TooltipWidget->AddToViewport(5);
            TooltipWidget->Hide();
        }
    }

    if (!TooltipWidget) return;

    if (NewItem && NewItem->ItemDefinition && IsValid(NewItem))
    {
        // Показать подсказку если смотрим на новый предмет
        if (LookedAtItem != NewItem)
        {
            LookedAtItem = NewItem;
            TooltipWidget->SetItemDefinition(NewItem->ItemDefinition, NewItem->Quantity);
        }
    }
    else
    {
        // Скрыть если не смотрим на предмет или предмет уничтожен
        if (LookedAtItem.IsValid() || TooltipWidget->GetVisibility() != ESlateVisibility::Collapsed)
        {
            LookedAtItem = nullptr;
            TooltipWidget->Hide();
        }
    }
}
