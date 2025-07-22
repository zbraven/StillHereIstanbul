#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/SHIItemData.h"
#include "SHIConsumablesHotbarWidget.generated.h"

class ASHICharacter;
class USHIConsumableSlotWidget;
class UHorizontalBox;

USTRUCT(BlueprintType)
struct FConsumableSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USHIItemData* ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    FConsumableSlotData()
    {
        ItemData = nullptr;
        Quantity = 0;
    }

    bool IsEmpty() const { return ItemData == nullptr || Quantity <= 0; }
};

UCLASS()
class STILLHEREISTANBUL_API USHIConsumablesHotbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    USHIConsumablesHotbarWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

    // Hotbar slot widgets (4 slots for keys 3,4,5,6)
    UPROPERTY(meta = (BindWidget))
    USHIConsumableSlotWidget* Slot3Widget;

    UPROPERTY(meta = (BindWidget))
    USHIConsumableSlotWidget* Slot4Widget;

    UPROPERTY(meta = (BindWidget))
    USHIConsumableSlotWidget* Slot5Widget;

    UPROPERTY(meta = (BindWidget))
    USHIConsumableSlotWidget* Slot6Widget;

    // Owner character reference
    UPROPERTY()
    ASHICharacter* OwnerCharacter;

    // Hotbar data (slots 3,4,5,6)
    UPROPERTY()
    TArray<FConsumableSlotData> HotbarSlots;

public:
    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void SetOwnerCharacter(ASHICharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void SetSlotItem(int32 SlotIndex, USHIItemData* Item, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void UseSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void RefreshSlotDisplay(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
    void RefreshAllSlots();

    // Consumable effect application
    void ApplyConsumableEffect(USHIItemData* Item);

private:
    // Helper functions
    USHIConsumableSlotWidget* GetSlotWidget(int32 SlotIndex);
    bool IsValidSlotIndex(int32 SlotIndex) const;
};