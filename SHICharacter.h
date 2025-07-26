#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/SHIStatsComponent.h"
#include "Components/SHIInventoryComponent.h"
#include "Components/SHIEquipmentComponent.h"
#include "Components/SHIAbilityComponent.h"
#include "Data/SHIItemData.h"
#include "Systems/SHIWorldItem.h"
#include "UI/SHIEquipmentPanelWidget.h"
#include "UI/SHIConsumablesHotbarWidget.h"
#include "SHICharacter.generated.h"

// Forward declarations for classes we only use as pointers
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class USHICharacterStatsWidget;
class USHIInventoryWidget;

UCLASS()
class STILLHEREISTANBUL_API ASHICharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASHICharacter();

protected:
    // UE5.6 Enhanced Input System
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    // SHI specific actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* TestStatsAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* TestItemsAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* TestSpawnAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ToggleStatsAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ToggleInventoryAction;

    // Equipment and weapon actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SwitchWeapon1Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SwitchWeapon2Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ToggleEquipmentAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ToggleEquipmentPanelAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* TestEquipAction;

    // Consumable actions (3,4,5,6 keys)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseConsumable3Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseConsumable4Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseConsumable5Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseConsumable6Action;

    // ⬅️ NEW: Test actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* TestPopulateHotbarAction; // Y key for hotbar test

    // Combat Ability Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseQAbilityAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseRAbilityAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseFAbilityAction;

    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    // SHI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SHI Components")
    USHIStatsComponent* StatsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SHI Components")
    USHIInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SHI Components")
    USHIEquipmentComponent* EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SHI Components")
    USHIAbilityComponent* AbilityComponent;

    // UI Components
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<USHICharacterStatsWidget> StatsWidgetClass;

    UPROPERTY()
    USHICharacterStatsWidget* StatsWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<USHIInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    USHIInventoryWidget* InventoryWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<USHIEquipmentPanelWidget> EquipmentPanelWidgetClass;

    UPROPERTY()
    USHIEquipmentPanelWidget* EquipmentPanelWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<USHIConsumablesHotbarWidget> ConsumablesHotbarWidgetClass;

    UPROPERTY()
    USHIConsumablesHotbarWidget* ConsumablesHotbarWidget;

    // Test Items for development
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestSwordItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestPotionItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestMaterialItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestShieldItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestHelmetItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test Items")
    USHIItemData* TestArmorItem;

    // Enhanced: Test Shield for shield logic testing (uses existing TestShieldItem)

    // ⬅️ NEW: Test items for spawning and consumables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SHI Debug")
    TArray<USHIItemData*> TestConsumableItems; // Consumable test items

    // Test item cycling
    UPROPERTY()
    int32 CurrentTestItemIndex = 0;

    // Interaction system
    UPROPERTY()
    ASHIWorldItem* NearbyWorldItem = nullptr;

    UPROPERTY()
    int32 SpawnItemIndex = 0;

    // Equipment test cycling
    UPROPERTY()
    int32 CurrentTestEquipIndex = 0;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Component getters
    UFUNCTION(BlueprintPure, Category = "SHI Stats")
    USHIStatsComponent* GetStatsComponent() const { return StatsComponent; }

    UFUNCTION(BlueprintPure, Category = "SHI Inventory")
    USHIInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    UFUNCTION(BlueprintPure, Category = "SHI Equipment")
    USHIEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

    UFUNCTION(BlueprintPure, Category = "SHI Abilities")
    USHIAbilityComponent* GetAbilityComponent() const { return AbilityComponent; }

    // Consumables hotbar functions
    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseConsumableSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables") 
    void SetHotbarSlot(int32 SlotIndex, USHIItemData* Item, int32 Quantity);

protected:
    // UE5.6 Enhanced Input Functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // SHI specific functions
    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestStatIncrease();

    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestAddItem();

    UFUNCTION(BlueprintCallable, Category = "SHI Interaction")
    void InteractWithItem();

    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestSpawnItem();

    UFUNCTION(BlueprintCallable, Category = "SHI UI")
    void ToggleStatsDisplay();

    UFUNCTION(BlueprintCallable, Category = "SHI UI")
    void ToggleInventoryDisplay();

    UFUNCTION(BlueprintCallable, Category = "SHI UI")
    void ToggleEquipmentDisplay();

    UFUNCTION(BlueprintCallable, Category = "SHI UI")
    void ToggleEquipmentPanel();

    // ⬅️ NEW: SHI specific functions (mevcut kodların altına)
    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestPopulateHotbar(); // Hotbar test function

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
    void SwitchToWeapon1();

    UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
    void SwitchToWeapon2();

    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestEquipItem();

    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void TestEquipShieldDirect();

    // Consumable functions
    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseConsumableSlot3();

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseConsumableSlot4();

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseConsumableSlot5();

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseConsumableSlot6();

    // Combat ability functions
    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseQAbility();

    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseRAbility();

    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseFAbility();

    UFUNCTION(BlueprintCallable, Category = "SHI Equipment")
    void OnActiveWeaponChanged_Abilities(ESHIEquipmentSlot NewActiveWeapon);

    // Network functions
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_TestStatIncrease();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_TestAddItem();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_InteractWithItem();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_TestSpawnItem();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_ToggleStatsDisplay();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_ToggleInventoryDisplay();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_TestEquipItem();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_UseConsumableSlot(int32 SlotNumber);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_ToggleEquipmentPanel();

    // Equipment event handlers
    UFUNCTION()
    void OnEquipmentChanged(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem);

    UFUNCTION()
    void OnActiveWeaponChanged(ESHIEquipmentSlot NewActiveWeapon);

    // Stats recalculation
    void RecalculateStatsFromEquipment();

    // Debug function
    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void DebugPrintAllSystems() const;

public:
    // Returns CameraBoom subobject
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    // Returns FollowCamera subobject
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};