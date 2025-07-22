#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SHIInventoryComponent.h"
#include "SHIInventorySlotWidget.generated.h"

class USHIInventoryWidget;

UCLASS()
class STILLHEREISTANBUL_API USHIInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // Widget References (Bind these in Blueprint)
    UPROPERTY(meta = (BindWidget))
    UImage* SlotBackground;
    
    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* QuantityText;
    
    UPROPERTY(meta = (BindWidget))
    UButton* SlotButton;

    // Slot properties
    UPROPERTY()
    int32 SlotIndex = -1;
    
    UPROPERTY()
    FSHIInventorySlot CurrentSlotData;
    
    UPROPERTY()
    USHIInventoryWidget* OwnerInventory;

    // Visual settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Settings")
    FLinearColor EmptySlotColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Settings")
    FLinearColor FilledSlotColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Settings")
    FLinearColor HoverSlotColor = FLinearColor(0.6f, 0.6f, 0.2f, 1.0f);

public:
    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetSlotIndex(int32 InSlotIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetOwnerInventory(USHIInventoryWidget* InOwnerInventory);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void UpdateSlotData(const FSHIInventorySlot& SlotData);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Inventory Slot")
    int32 GetSlotIndex() const { return SlotIndex; }
    
    UFUNCTION(BlueprintPure, Category = "Inventory Slot")
    bool IsSlotEmpty() const { return CurrentSlotData.IsEmpty(); }

    UFUNCTION(BlueprintPure, Category = "Inventory Slot")
    FSHIInventorySlot GetSlotData() const { return CurrentSlotData; }

    // UI State functions
    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetSlotHighlighted(bool bHighlighted);

protected:
    // UI update functions
    void UpdateSlotVisuals();
    void SetSlotEmpty();
    void SetSlotWithItem();

    // Event handlers
    UFUNCTION()
    void OnSlotClicked();

    UFUNCTION()
    void OnSlotHovered();

    UFUNCTION()
    void OnSlotUnhovered();
};