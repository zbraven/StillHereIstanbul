#include "SHICharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "UI/SHICharacterStatsWidget.h"
#include "UI/SHIInventoryWidget.h"

ASHICharacter::ASHICharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->SetCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Create a camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Create SHI Stats Component
    StatsComponent = CreateDefaultSubobject<USHIStatsComponent>(TEXT("StatsComponent"));

    // Create SHI Inventory Component
    InventoryComponent = CreateDefaultSubobject<USHIInventoryComponent>(TEXT("InventoryComponent"));

    // Create SHI Equipment Component
    EquipmentComponent = CreateDefaultSubobject<USHIEquipmentComponent>(TEXT("EquipmentComponent"));

    // Initialize properties
    CurrentTestItemIndex = 0;
    SpawnItemIndex = 0;
    CurrentTestEquipIndex = 0;
    NearbyWorldItem = nullptr;
    StatsWidget = nullptr;
    InventoryWidget = nullptr;
    EquipmentPanelWidget = nullptr;
    ConsumablesHotbarWidget = nullptr;
}

void ASHICharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Istanbul flavor log - Character stats
    if (StatsComponent)
    {
        float CurrentGuc = StatsComponent->GetCurrentGuc();
        UE_LOG(LogTemp, Warning, TEXT("Istanbul'da bir karakter doğdu! Güç: %f"), CurrentGuc);
    }

    // Inventory initialization log
    if (InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Karakter envanteri hazır! Slot sayısı: %d"), InventoryComponent->InventorySize);
    }

    // Equipment initialization log
    if (EquipmentComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Karakter equipment sistemi hazır! 8 slot aktif"));
        
        // Bind equipment events
        EquipmentComponent->OnEquipmentChanged.AddDynamic(this, &ASHICharacter::OnEquipmentChanged);
        EquipmentComponent->OnActiveWeaponChanged.AddDynamic(this, &ASHICharacter::OnActiveWeaponChanged);
    }

    // Initialize consumables hotbar on client
    if (IsLocallyControlled() && ConsumablesHotbarWidgetClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            ConsumablesHotbarWidget = CreateWidget<USHIConsumablesHotbarWidget>(PC, ConsumablesHotbarWidgetClass);
            if (ConsumablesHotbarWidget)
            {
                ConsumablesHotbarWidget->SetOwnerCharacter(this);
                ConsumablesHotbarWidget->AddToViewport();
                
                UE_LOG(LogTemp, Log, TEXT("Consumables hotbar initialized"));
                
                // Add some test consumables for testing
                if (TestPotionItem)
                {
                    ConsumablesHotbarWidget->SetSlotItem(3, TestPotionItem, 5);
                    ConsumablesHotbarWidget->SetSlotItem(4, TestPotionItem, 3);
                    UE_LOG(LogTemp, Log, TEXT("Test consumables added to hotbar"));
                }
            }
        }
    }

    // ⬅️ NEW: Initialize test consumable items if not set in Blueprint
    if (TestConsumableItems.Num() == 0)
    {
        // Create default test consumables
        UE_LOG(LogTemp, Warning, TEXT("Test consumable items not configured in Blueprint - using defaults"));
        UE_LOG(LogTemp, Log, TEXT("Use Y key to populate hotbar with test consumables"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Press Y to populate hotbar with test items"));
        }
    }

    // Test items validation
    UE_LOG(LogTemp, Log, TEXT("Test Items Status:"));
    UE_LOG(LogTemp, Log, TEXT(" Sword: %s"), TestSwordItem ? TEXT("Loaded") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Potion: %s"), TestPotionItem ? TEXT("Loaded") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Material: %s"), TestMaterialItem ? TEXT("Loaded") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Shield: %s"), TestShieldItem ? TEXT("Loaded") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Helmet: %s"), TestHelmetItem ? TEXT("Loaded") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Armor: %s"), TestArmorItem ? TEXT("Loaded") : TEXT("NULL"));

    // UI validation
    UE_LOG(LogTemp, Log, TEXT("UI Widget Classes:"));
    UE_LOG(LogTemp, Log, TEXT(" Stats Widget Class: %s"), StatsWidgetClass ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Inventory Widget Class: %s"), InventoryWidgetClass ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Equipment Panel Widget Class: %s"), EquipmentPanelWidgetClass ? TEXT("Set") : TEXT("NULL"));
    UE_LOG(LogTemp, Log, TEXT(" Consumables Hotbar Widget Class: %s"), ConsumablesHotbarWidgetClass ? TEXT("Set") : TEXT("NULL"));
}

void ASHICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASHICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASHICharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASHICharacter::Look);

        // SHI Test Stats (T key)
        EnhancedInputComponent->BindAction(TestStatsAction, ETriggerEvent::Triggered, this, &ASHICharacter::TestStatIncrease);

        // SHI Test Items (I key)
        EnhancedInputComponent->BindAction(TestItemsAction, ETriggerEvent::Triggered, this, &ASHICharacter::TestAddItem);

        // SHI Interact (E key)
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASHICharacter::InteractWithItem);

        // SHI Test Spawn (P key)
        EnhancedInputComponent->BindAction(TestSpawnAction, ETriggerEvent::Triggered, this, &ASHICharacter::TestSpawnItem);

        // SHI Toggle Stats (H key)
        EnhancedInputComponent->BindAction(ToggleStatsAction, ETriggerEvent::Triggered, this, &ASHICharacter::ToggleStatsDisplay);

        // SHI Toggle Inventory (B key)
        EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Triggered, this, &ASHICharacter::ToggleInventoryDisplay);

        // SHI Toggle Equipment (K key) - OLD PLACEHOLDER
        EnhancedInputComponent->BindAction(ToggleEquipmentAction, ETriggerEvent::Triggered, this, &ASHICharacter::ToggleEquipmentDisplay);

        // SHI Toggle Equipment Panel (K key) - ACTUAL IMPLEMENTATION
        EnhancedInputComponent->BindAction(ToggleEquipmentPanelAction, ETriggerEvent::Triggered, this, &ASHICharacter::ToggleEquipmentPanel);

        // Weapon Switching (1/2 keys)
        EnhancedInputComponent->BindAction(SwitchWeapon1Action, ETriggerEvent::Triggered, this, &ASHICharacter::SwitchToWeapon1);
        EnhancedInputComponent->BindAction(SwitchWeapon2Action, ETriggerEvent::Triggered, this, &ASHICharacter::SwitchToWeapon2);

        // Test Equipment (U key)
        EnhancedInputComponent->BindAction(TestEquipAction, ETriggerEvent::Triggered, this, &ASHICharacter::TestEquipItem);

        // SHI Consumables (3,4,5,6 keys) - existing
        EnhancedInputComponent->BindAction(UseConsumable3Action, ETriggerEvent::Triggered, this, &ASHICharacter::UseConsumableSlot3);
        EnhancedInputComponent->BindAction(UseConsumable4Action, ETriggerEvent::Triggered, this, &ASHICharacter::UseConsumableSlot4);
        EnhancedInputComponent->BindAction(UseConsumable5Action, ETriggerEvent::Triggered, this, &ASHICharacter::UseConsumableSlot5);
        EnhancedInputComponent->BindAction(UseConsumable6Action, ETriggerEvent::Triggered, this, &ASHICharacter::UseConsumableSlot6);

        // ⬅️ NEW: Hotbar test key (Y tuşu)
        if (TestPopulateHotbarAction)
        {
            EnhancedInputComponent->BindAction(TestPopulateHotbarAction, ETriggerEvent::Triggered, this, &ASHICharacter::TestPopulateHotbar);
        }
    }
}

void ASHICharacter::Move(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // Get right vector
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ASHICharacter::Look(const FInputActionValue& Value)
{
    // Input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ASHICharacter::TestStatIncrease()
{
    // Client-side call, forward to server
    Server_TestStatIncrease();
}

void ASHICharacter::TestAddItem()
{
    // Client-side call, forward to server
    Server_TestAddItem();
}

void ASHICharacter::InteractWithItem()
{
    // Client-side call, forward to server
    Server_InteractWithItem();
}

void ASHICharacter::TestSpawnItem()
{
    // Client-side call, forward to server
    Server_TestSpawnItem();
}

void ASHICharacter::ToggleStatsDisplay()
{
    // Handle this client-side for UI responsiveness
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsLocalController())
        {
            if (!StatsWidget && StatsWidgetClass)
            {
                StatsWidget = CreateWidget<USHICharacterStatsWidget>(PC, StatsWidgetClass);
                if (StatsWidget)
                {
                    StatsWidget->SetOwnerCharacter(this);
                    StatsWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Stats widget created and added to viewport"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Stats HUD Açıldı"));
                    }
                }
            }
            else if (StatsWidget)
            {
                if (StatsWidget->IsInViewport())
                {
                    StatsWidget->RemoveFromParent(); // UE5 Modern syntax
                    UE_LOG(LogTemp, Log, TEXT("Stats widget hidden"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Stats HUD Kapatıldı"));
                    }
                }
                else
                {
                    StatsWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Stats widget shown"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Stats HUD Açıldı"));
                    }
                }
            }
        }
    }
}

void ASHICharacter::ToggleInventoryDisplay()
{
    // Handle client-side for UI responsiveness
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsLocalController())
        {
            if (!InventoryWidget && InventoryWidgetClass)
            {
                InventoryWidget = CreateWidget<USHIInventoryWidget>(PC, InventoryWidgetClass);
                if (InventoryWidget)
                {
                    InventoryWidget->SetOwnerCharacter(this);
                    InventoryWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Inventory widget created and added to viewport"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Envanter Açıldı"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to create inventory widget"));
                }
            }
            else if (InventoryWidget)
            {
                if (InventoryWidget->IsInViewport())
                {
                    InventoryWidget->RemoveFromParent(); // UE5 Modern syntax
                    UE_LOG(LogTemp, Log, TEXT("Inventory widget hidden"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Envanter Kapatıldı"));
                    }
                }
                else
                {
                    InventoryWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Inventory widget shown"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Envanter Açıldı"));
                    }
                }
            }
        }
    }
}

void ASHICharacter::ToggleEquipmentDisplay()
{
    // OLD PLACEHOLDER - Keep for backward compatibility if needed
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Equipment Display - Use K for Equipment Panel!"));
    }
    UE_LOG(LogTemp, Log, TEXT("Equipment display toggle - redirecting to Equipment Panel"));
}

void ASHICharacter::ToggleEquipmentPanel()
{
    // Handle client-side for UI responsiveness
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsLocalController())
        {
            if (!EquipmentPanelWidget && EquipmentPanelWidgetClass)
            {
                EquipmentPanelWidget = CreateWidget<USHIEquipmentPanelWidget>(PC, EquipmentPanelWidgetClass);
                if (EquipmentPanelWidget)
                {
                    EquipmentPanelWidget->SetOwnerCharacter(this);
                    EquipmentPanelWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Equipment panel created and shown"));
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Equipment Panel Açıldı"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to create equipment panel widget"));
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Equipment Panel Widget Class ayarlanmamış!"));
                    }
                }
            }
            else if (EquipmentPanelWidget)
            {
                if (EquipmentPanelWidget->IsInViewport())
                {
                    EquipmentPanelWidget->RemoveFromParent();
                    UE_LOG(LogTemp, Log, TEXT("Equipment panel hidden"));
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Equipment Panel Kapatıldı"));
                    }
                }
                else
                {
                    EquipmentPanelWidget->AddToViewport();
                    UE_LOG(LogTemp, Log, TEXT("Equipment panel shown"));
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Equipment Panel Açıldı"));
                    }
                }
            }
        }
    }
}

void ASHICharacter::SwitchToWeapon1()
{
    if (EquipmentComponent)
    {
        EquipmentComponent->Server_SetActiveWeapon(ESHIEquipmentSlot::Silah1);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Silah-1 Aktif"));
        }
    }
}

void ASHICharacter::SwitchToWeapon2()
{
    if (EquipmentComponent)
    {
        EquipmentComponent->Server_SetActiveWeapon(ESHIEquipmentSlot::Silah2);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Silah-2 Aktif"));
        }
    }
}

void ASHICharacter::TestEquipItem()
{
    // Client-side call, forward to server
    Server_TestEquipItem();
}

// ⬅️ NEW: File sonuna TestPopulateHotbar implementation ekle
void ASHICharacter::TestPopulateHotbar()
{
    UE_LOG(LogTemp, Warning, TEXT("Populating hotbar with test consumables"));

    // Test consumable items (hardcoded for now)
    TArray<FString> TestConsumableNames = {
        TEXT("Health Potion"),
        TEXT("Mana Potion"), 
        TEXT("Stamina Potion"),
        TEXT("Speed Potion")
    };

    // Create test consumable items and add to hotbar
    for (int32 i = 0; i < 4; i++)
    {
        int32 SlotIndex = i + 3; // Slots 3,4,5,6
        
        // Create a temporary test consumable item
        USHIItemData* TestConsumable = NewObject<USHIItemData>();
        if (TestConsumable)
        {
            TestConsumable->ItemName = FText::FromString(TestConsumableNames[i]);
            TestConsumable->ItemType = ESHIItemType::Tuketim;
            TestConsumable->ItemDescription = FText::FromString(FString::Printf(TEXT("Test %s for hotbar slot %d"), *TestConsumableNames[i], SlotIndex));
            TestConsumable->MaxStackSize = 10;
            
            // Add to hotbar with quantity 5
            SetHotbarSlot(SlotIndex, TestConsumable, 5);
            
            UE_LOG(LogTemp, Log, TEXT("Added %s to hotbar slot %d"), *TestConsumableNames[i], SlotIndex);
        }
    }

    // Visual feedback
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, TEXT("Hotbar Populated with Test Items!"));
    }

    // Also show usage instructions
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Use Keys 3,4,5,6 to consume items"));
    }
}

// ⬅️ UPDATED: Ayrıca UseConsumableSlot3-6 functions'ları güncelle
void ASHICharacter::UseConsumableSlot3()
{
    UseConsumableSlot(3);
}

void ASHICharacter::UseConsumableSlot4()
{
    UseConsumableSlot(4);
}

void ASHICharacter::UseConsumableSlot5()
{
    UseConsumableSlot(5);
}

void ASHICharacter::UseConsumableSlot6()
{
    UseConsumableSlot(6);
}

void ASHICharacter::OnEquipmentChanged(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem)
{
    // Handle equipment changes
    if (NewItem)
    {
        UE_LOG(LogTemp, Log, TEXT("Equipment changed in slot %d: %s equipped"), 
               (int32)SlotType, *NewItem->ItemName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Equipment removed from slot %d"), (int32)SlotType);
    }

    // Recalculate stats when equipment changes
    RecalculateStatsFromEquipment();
}

void ASHICharacter::OnActiveWeaponChanged(ESHIEquipmentSlot NewActiveWeapon)
{
    USHIItemData* ActiveWeapon = EquipmentComponent->GetActiveWeapon();
    
    if (ActiveWeapon)
    {
        UE_LOG(LogTemp, Log, TEXT("Active weapon changed to: %s (Slot %d)"), 
               *ActiveWeapon->ItemName.ToString(), (int32)NewActiveWeapon);
               
        if (GEngine)
        {
            FString WeaponText = FString::Printf(TEXT("Aktif Silah: %s"), 
                                               *ActiveWeapon->ItemName.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, WeaponText);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No active weapon equipped"));
    }
}

void ASHICharacter::RecalculateStatsFromEquipment()
{
    if (!StatsComponent || !EquipmentComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing components for stats recalculation"));
        return;
    }

    // Get all equipment stat bonuses
    TArray<FSHIStatModifier> EquipmentBonuses = EquipmentComponent->GetAllEquipmentStatBonuses();
    
    UE_LOG(LogTemp, Log, TEXT("Recalculating stats from equipment: %d bonuses found"), EquipmentBonuses.Num());
    
    // FIX: ACTUALLY APPLY THE BONUSES TO STATS COMPONENT
    StatsComponent->ApplyEquipmentBonuses(EquipmentBonuses);
    
    // Log the results for debugging
    for (const FSHIStatModifier& Bonus : EquipmentBonuses)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Applied equipment bonus: %s +%f"), 
            *Bonus.StatName.ToString(), Bonus.BonusAmount);
    }
    
    // Show updated stats on screen for testing
    if (GEngine)
    {
        FString StatsText = FString::Printf(TEXT("Stats Updated! Güç: %.0f, Çeviklik: %.0f, Zeka: %.0f"), 
            StatsComponent->GetCurrentGuc(), 
            StatsComponent->GetCurrentCeviklik(), 
            StatsComponent->GetCurrentZeka());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, StatsText);
        
        // Also show base vs current comparison
        FString ComparisonText = FString::Printf(TEXT("Base Güç: %.0f | Current Güç: %.0f"), 
            StatsComponent->GetBaseStats().Guc, 
            StatsComponent->GetCurrentGuc());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, ComparisonText);
    }
}

// Consumables Functions Implementation
void ASHICharacter::UseConsumableSlot(int32 SlotIndex)
{
    // Validate slot index
    if (SlotIndex < 3 || SlotIndex > 6)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid consumable slot index: %d"), SlotIndex);
        return;
    }

    // Use through hotbar widget
    if (ConsumablesHotbarWidget)
    {
        ConsumablesHotbarWidget->UseSlot(SlotIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Consumables hotbar widget not found"));
    }

    // Server call for validation/sync
    Server_UseConsumableSlot(SlotIndex);

    // Visual feedback
    if (GEngine)
    {
        FString UseText = FString::Printf(TEXT("Consumable Slot %d Used"), SlotIndex);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, UseText);
    }
}

void ASHICharacter::SetHotbarSlot(int32 SlotIndex, USHIItemData* Item, int32 Quantity)
{
    if (ConsumablesHotbarWidget)
    {
        ConsumablesHotbarWidget->SetSlotItem(SlotIndex, Item, Quantity);
        
        UE_LOG(LogTemp, Log, TEXT("Hotbar slot %d set to: %s x%d"), 
               SlotIndex, Item ? *Item->ItemName.ToString() : TEXT("Empty"), Quantity);
    }
}

// =============================================================================
// SERVER IMPLEMENTATIONS - COMPLETE
// =============================================================================

void ASHICharacter::Server_TestStatIncrease_Implementation()
{
    if (StatsComponent)
    {
        // Increase Güç by 2 each time T is pressed
        StatsComponent->Server_ModifyBaseStat(FName("Guc"), 2.0f);

        // Get current values
        float CurrentGuc = StatsComponent->GetCurrentGuc();
        float MaxSaglik = StatsComponent->GetMaxSaglik();

        // Show current stats in log
        UE_LOG(LogTemp, Warning, TEXT("Stats artırıldı! Yeni Güç: %f, Max Sağlık: %f"), CurrentGuc, MaxSaglik);

        // Print to screen for visual feedback - UE5.6 compatible
        if (GEngine)
        {
            FString StatsText = FString::Printf(TEXT("Güç: %.0f | Sağlık: %.0f"), CurrentGuc, MaxSaglik);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, StatsText);
        }
    }
}

void ASHICharacter::Server_TestAddItem_Implementation()
{
    if (InventoryComponent)
    {
        // Test items array
        TArray<USHIItemData*> TestItems = {TestSwordItem, TestPotionItem, TestMaterialItem};

        // Cycle through test items
        if (TestItems.Num() > 0)
        {
            USHIItemData* CurrentItem = TestItems[CurrentTestItemIndex % TestItems.Num()];
            if (CurrentItem)
            {
                // Add different quantities based on item type
                int32 AddQuantity = 1;
                if (CurrentItem->ItemType == ESHIItemType::Tuketim)
                {
                    AddQuantity = 3; // Add 3 potions
                }
                else if (CurrentItem->ItemType == ESHIItemType::Materyal)
                {
                    AddQuantity = 10; // Add 10 materials
                }

                // Check if we can add the item
                if (InventoryComponent->CanAddItem(CurrentItem, AddQuantity))
                {
                    InventoryComponent->Server_AddItem(CurrentItem, AddQuantity);
                    UE_LOG(LogTemp, Warning, TEXT("Added %d of %s to inventory"),
                           AddQuantity, *CurrentItem->ItemName.ToString());

                    // Screen feedback
                    if (GEngine)
                    {
                        FString ItemText = FString::Printf(TEXT("Eklendi: %s x%d"),
                                                         *CurrentItem->ItemName.ToString(), AddQuantity);
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, ItemText);
                    }

                    // Move to next item for next test
                    CurrentTestItemIndex++;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cannot add %s - inventory full or insufficient space"),
                           *CurrentItem->ItemName.ToString());
                    if (GEngine)
                    {
                        FString ErrorText = FString::Printf(TEXT("Envanter dolu: %s"),
                                                          *CurrentItem->ItemName.ToString());
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, ErrorText);
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Test item %d is null - check Blueprint configuration"), CurrentTestItemIndex);
                CurrentTestItemIndex++; // Skip null item
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No test items configured"));
        }

        // Show inventory status
        int32 EmptySlot = InventoryComponent->FindFirstEmptySlot();
        int32 TotalSlots = InventoryComponent->InventorySize;
        int32 UsedSlots = 0;

        // Count used slots
        for (int32 i = 0; i < TotalSlots; i++)
        {
            if (!InventoryComponent->GetSlot(i).IsEmpty())
            {
                UsedSlots++;
            }
        }

        if (GEngine)
        {
            FString InventoryText = FString::Printf(TEXT("Envanter: %d/%d dolu"), UsedSlots, TotalSlots);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, InventoryText);
        }

        // Debug: Show all inventory contents
        UE_LOG(LogTemp, Log, TEXT("=== INVENTORY DEBUG ==="));
        for (int32 i = 0; i < InventoryComponent->InventorySize; i++)
        {
            FSHIInventorySlot Slot = InventoryComponent->GetSlot(i);
            if (!Slot.IsEmpty())
            {
                UE_LOG(LogTemp, Log, TEXT("Slot %d: %s x%d (Type: %s)"),
                       i,
                       *Slot.ItemData->ItemName.ToString(),
                       Slot.Quantity,
                       *Slot.ItemData->GetLocalizedTypeName().ToString());
            }
        }
        UE_LOG(LogTemp, Log, TEXT("=== END INVENTORY DEBUG ==="));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Inventory component not found!"));
    }
}

void ASHICharacter::Server_InteractWithItem_Implementation()
{
    // Find nearby world item using overlap detection
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, ASHIWorldItem::StaticClass());

    if (OverlappingActors.Num() > 0)
    {
        // Find the closest world item
        ASHIWorldItem* ClosestWorldItem = nullptr;
        float ClosestDistance = FLT_MAX;

        for (AActor* Actor : OverlappingActors)
        {
            if (ASHIWorldItem* WorldItem = Cast<ASHIWorldItem>(Actor))
            {
                float Distance = FVector::Dist(GetActorLocation(), WorldItem->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestWorldItem = WorldItem;
                }
            }
        }

        // Try to pick up the closest item
        if (ClosestWorldItem)
        {
            if (ClosestWorldItem->TryPickupItem(this))
            {
                UE_LOG(LogTemp, Log, TEXT("Successfully picked up item: %s"),
                       *ClosestWorldItem->GetItemData()->ItemName.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to pick up item: %s"),
                       *ClosestWorldItem->GetItemData()->ItemName.ToString());
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No world items in range to interact with"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Yakında alınabilir eşya yok"));
        }
    }
}

void ASHICharacter::Server_TestSpawnItem_Implementation()
{
    // Test items array
    TArray<USHIItemData*> TestItems = {TestSwordItem, TestPotionItem, TestMaterialItem};

    if (TestItems.Num() > 0)
    {
        USHIItemData* ItemToSpawn = TestItems[SpawnItemIndex % TestItems.Num()];
        if (ItemToSpawn)
        {
            // Spawn location in front of character
            FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 200.0f;
            SpawnLocation.Z += 50.0f; // Slightly above ground
            FRotator SpawnRotation = FRotator::ZeroRotator;

            // Spawn the world item
            ASHIWorldItem* WorldItem = GetWorld()->SpawnActor<ASHIWorldItem>(ASHIWorldItem::StaticClass(), SpawnLocation, SpawnRotation);
            if (WorldItem)
            {
                // Set different quantities for different item types
                int32 SpawnQuantity = 1;
                if (ItemToSpawn->ItemType == ESHIItemType::Tuketim)
                {
                    SpawnQuantity = 3;
                }
                else if (ItemToSpawn->ItemType == ESHIItemType::Materyal)
                {
                    SpawnQuantity = 5;
                }

                WorldItem->InitializeWorldItem(ItemToSpawn, SpawnQuantity);

                UE_LOG(LogTemp, Log, TEXT("Spawned world item: %s x%d at location %s"),
                       *ItemToSpawn->ItemName.ToString(), SpawnQuantity, *SpawnLocation.ToString());

                if (GEngine)
                {
                    FString SpawnText = FString::Printf(TEXT("Spawn: %s x%d"),
                                                      *ItemToSpawn->ItemName.ToString(), SpawnQuantity);
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, SpawnText);
                }

                SpawnItemIndex++;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn world item"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Test item to spawn is null"));
            SpawnItemIndex++; // Skip null item
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No test items configured for spawning"));
    }
}

void ASHICharacter::Server_TestEquipItem_Implementation()
{
    if (!EquipmentComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Equipment component not found!"));
        return;
    }

    // Test equipment items array
    TArray<USHIItemData*> TestEquipmentItems = {
        TestSwordItem, TestShieldItem, TestHelmetItem, 
        TestArmorItem, TestSwordItem // Add sword twice for weapon switching test
    };

    TArray<ESHIEquipmentSlot> TestSlots = {
        ESHIEquipmentSlot::Silah1, ESHIEquipmentSlot::Kalkan, ESHIEquipmentSlot::Kask,
        ESHIEquipmentSlot::GoguslukZirhi, ESHIEquipmentSlot::Silah2
    };

    if (TestEquipmentItems.Num() > 0 && TestSlots.Num() > 0)
    {
        int32 ItemIndex = CurrentTestEquipIndex % TestEquipmentItems.Num();
        int32 SlotIndex = CurrentTestEquipIndex % TestSlots.Num();
        
        USHIItemData* ItemToEquip = TestEquipmentItems[ItemIndex];
        ESHIEquipmentSlot SlotToEquip = TestSlots[SlotIndex];

        if (ItemToEquip)
        {
            // Check if we can equip this item
            if (EquipmentComponent->CanEquipItem(ItemToEquip, SlotToEquip))
            {
                EquipmentComponent->Server_EquipItem(SlotToEquip, ItemToEquip, 1);
                
                UE_LOG(LogTemp, Log, TEXT("Test equipped %s in slot %d"), 
                       *ItemToEquip->ItemName.ToString(), (int32)SlotToEquip);
                       
                if (GEngine)
                {
                    FString EquipText = FString::Printf(TEXT("Equipped: %s"), 
                                                      *ItemToEquip->ItemName.ToString());
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, EquipText);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot equip %s in slot %d"), 
                       *ItemToEquip->ItemName.ToString(), (int32)SlotToEquip);
                       
                if (GEngine)
                {
                    FString ErrorText = FString::Printf(TEXT("Cannot equip: %s"), 
                                                      *ItemToEquip->ItemName.ToString());
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, ErrorText);
                }
            }
            
            CurrentTestEquipIndex++;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Test equipment item %d is null"), ItemIndex);
            CurrentTestEquipIndex++; // Skip null item
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No test equipment items configured"));
    }

    // Debug print equipment status
    if (EquipmentComponent)
    {
        EquipmentComponent->DebugPrintEquipment();
    }
}

void ASHICharacter::Server_UseConsumableSlot_Implementation(int32 SlotNumber)
{
    // Server-side validation and sync
    UE_LOG(LogTemp, Log, TEXT("Server: Consumable slot %d used"), SlotNumber);
    
    // Here you would:
    // 1. Validate the consumable usage
    // 2. Apply server-side effects
    // 3. Update client states
    
    // For now, just log
    if (GEngine)
    {
        FString ServerText = FString::Printf(TEXT("Server: Slot %d validated"), SlotNumber);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, ServerText);
    }
}

void ASHICharacter::Server_ToggleStatsDisplay_Implementation()
{
    // This function kept for consistency but stats display is handled client-side
    UE_LOG(LogTemp, Log, TEXT("Server received stats display toggle request"));
}

void ASHICharacter::Server_ToggleInventoryDisplay_Implementation()
{
    // This function kept for consistency but inventory display is handled client-side
    UE_LOG(LogTemp, Log, TEXT("Server received inventory display toggle request"));
}

void ASHICharacter::Server_ToggleEquipmentPanel_Implementation()
{
    // Consistency function, UI handled client-side
    UE_LOG(LogTemp, Log, TEXT("Server received equipment panel toggle request"));
}