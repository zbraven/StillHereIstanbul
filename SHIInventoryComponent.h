#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/SHIItemData.h"
#include "SHIInventoryComponent.generated.h"

// Inventory slot structure
USTRUCT(BlueprintType)
struct FSHIInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    USHIItemData* ItemData = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Quantity = 0;

    FSHIInventorySlot()
    {
        ItemData = nullptr;
        Quantity = 0;
    }
    
    bool IsEmpty() const
    {
        return ItemData == nullptr || Quantity <= 0;
    }
    
    bool CanStackWith(USHIItemData* OtherItem) const
    {
        return ItemData == OtherItem && ItemData && ItemData->IsStackable();
    }
};

// Events for UI updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, const FSHIInventorySlot&, NewSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, USHIItemData*, Item, int32, Quantity, int32, SlotIndex);

UCLASS(ClassGroup=(SHI), meta=(BlueprintSpawnableComponent))
class STILLHEREISTANBUL_API USHIInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USHIInventoryComponent();

    // Inventory properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Settings")
    int32 InventorySize = 30;  // Standard MMO inventory size
    
    UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, BlueprintReadOnly, Category = "Inventory")
    TArray<FSHIInventorySlot> InventorySlots;

    // Core inventory functions
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
    void Server_AddItem(USHIItemData* ItemData, int32 Quantity = 1);
    
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
    void Server_RemoveItem(int32 SlotIndex, int32 Quantity = 1);
    
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
    void Server_MoveItem(int32 FromSlot, int32 ToSlot);
    
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory")
    void Server_UseItem(int32 SlotIndex);

    // Query functions
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool CanAddItem(USHIItemData* ItemData, int32 Quantity = 1) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount(USHIItemData* ItemData) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FSHIInventorySlot GetSlot(int32 SlotIndex) const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 FindFirstEmptySlot() const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 FindItemSlot(USHIItemData* ItemData) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInventoryChanged OnInventoryChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnItemAdded OnItemAdded;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UFUNCTION()
    void OnRep_InventorySlots();
    
    // Internal helper functions
    void InitializeInventory();
    bool AddItemToSlot(int32 SlotIndex, USHIItemData* ItemData, int32 Quantity);
    void BroadcastSlotChange(int32 SlotIndex);
};