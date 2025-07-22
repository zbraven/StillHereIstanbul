#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/SHIItemData.h"
#include "SHIEquipmentComponent.generated.h"

// Equipment state for network replication (TMap yerine individual properties)
USTRUCT(BlueprintType)
struct FSHIEquipmentState
{
    GENERATED_BODY()

    // Individual equipment slots for network replication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot KaskSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot GoguslukZirhiSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot KalkanSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot Silah1Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot Silah2Slot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot KolyeSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot YuzukSlot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FSHIEquipmentSlot KupeSlot;
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    ESHIEquipmentSlot ActiveWeaponSlot = ESHIEquipmentSlot::Silah1;

    FSHIEquipmentState()
    {
        ActiveWeaponSlot = ESHIEquipmentSlot::Silah1;
        
        // All slots start empty (default constructor handles this)
        KaskSlot = FSHIEquipmentSlot();
        GoguslukZirhiSlot = FSHIEquipmentSlot();
        KalkanSlot = FSHIEquipmentSlot();
        Silah1Slot = FSHIEquipmentSlot();
        Silah2Slot = FSHIEquipmentSlot();
        KolyeSlot = FSHIEquipmentSlot();
        YuzukSlot = FSHIEquipmentSlot();
        KupeSlot = FSHIEquipmentSlot();
    }

    // Helper function to get slot by type
    FSHIEquipmentSlot* GetSlotByType(ESHIEquipmentSlot SlotType)
    {
        switch (SlotType)
        {
            case ESHIEquipmentSlot::Kask: return &KaskSlot;
            case ESHIEquipmentSlot::GoguslukZirhi: return &GoguslukZirhiSlot;
            case ESHIEquipmentSlot::Kalkan: return &KalkanSlot;
            case ESHIEquipmentSlot::Silah1: return &Silah1Slot;
            case ESHIEquipmentSlot::Silah2: return &Silah2Slot;
            case ESHIEquipmentSlot::Kolye: return &KolyeSlot;
            case ESHIEquipmentSlot::Yuzuk: return &YuzukSlot;
            case ESHIEquipmentSlot::Kupe: return &KupeSlot;
            default: return nullptr;
        }
    }

    // Const version
    const FSHIEquipmentSlot* GetSlotByType(ESHIEquipmentSlot SlotType) const
    {
        switch (SlotType)
        {
            case ESHIEquipmentSlot::Kask: return &KaskSlot;
            case ESHIEquipmentSlot::GoguslukZirhi: return &GoguslukZirhiSlot;
            case ESHIEquipmentSlot::Kalkan: return &KalkanSlot;
            case ESHIEquipmentSlot::Silah1: return &Silah1Slot;
            case ESHIEquipmentSlot::Silah2: return &Silah2Slot;
            case ESHIEquipmentSlot::Kolye: return &KolyeSlot;
            case ESHIEquipmentSlot::Yuzuk: return &YuzukSlot;
            case ESHIEquipmentSlot::Kupe: return &KupeSlot;
            default: return nullptr;
        }
    }
};

// Equipment change event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentChanged, ESHIEquipmentSlot, SlotType, USHIItemData*, NewItem, USHIItemData*, OldItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWeaponChanged, ESHIEquipmentSlot, NewActiveWeapon);

UCLASS(ClassGroup=(SHI), meta=(BlueprintSpawnableComponent))
class STILLHEREISTANBUL_API USHIEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USHIEquipmentComponent();

protected:
    // Network replicated equipment state
    UPROPERTY(ReplicatedUsing = OnRep_EquipmentState, BlueprintReadOnly, Category = "Equipment")
    FSHIEquipmentState EquipmentState;

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // Equipment management functions
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Equipment")
    void Server_EquipItem(ESHIEquipmentSlot SlotType, USHIItemData* ItemData, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Equipment")
    void Server_UnequipItem(ESHIEquipmentSlot SlotType);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Equipment")
    void Server_SetActiveWeapon(ESHIEquipmentSlot WeaponSlot);

    // Equipment query functions
    UFUNCTION(BlueprintPure, Category = "Equipment")
    FSHIEquipmentSlot GetEquippedItem(ESHIEquipmentSlot SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool IsSlotEmpty(ESHIEquipmentSlot SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    USHIItemData* GetActiveWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    ESHIEquipmentSlot GetActiveWeaponSlot() const { return EquipmentState.ActiveWeaponSlot; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool CanEquipItem(USHIItemData* ItemData, ESHIEquipmentSlot SlotType) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    TArray<FSHIStatModifier> GetAllEquipmentStatBonuses() const;

    // Shield dependency check (Kalkan requires Kılıç)
    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool IsShieldSlotAvailable() const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool HasSwordEquipped() const;

    // Equipment validation
    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool IsValidEquipmentSlot(ESHIEquipmentSlot SlotType, USHIItemData* ItemData) const;

    // Get all equipment slots as array (for UI iteration)
    UFUNCTION(BlueprintPure, Category = "Equipment")
    TArray<ESHIEquipmentSlot> GetAllEquipmentSlots() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Equipment Events")
    FOnEquipmentChanged OnEquipmentChanged;

    UPROPERTY(BlueprintAssignable, Category = "Equipment Events")  
    FOnActiveWeaponChanged OnActiveWeaponChanged;

protected:
    // Network replication
    UFUNCTION()
    void OnRep_EquipmentState();

    // Internal helper functions
    void ValidateShieldEquipment();
    void BroadcastEquipmentChange(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem);
    void BroadcastActiveWeaponChange();

    // Equipment slot validation
    bool IsWeaponSlot(ESHIEquipmentSlot SlotType) const;
    bool IsArmorSlot(ESHIEquipmentSlot SlotType) const;
    bool IsAccessorySlot(ESHIEquipmentSlot SlotType) const;

public:
    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Equipment Debug")
    void DebugPrintEquipment() const;

    UFUNCTION(BlueprintPure, Category = "Equipment Debug")
    int32 GetEquippedItemCount() const;
};