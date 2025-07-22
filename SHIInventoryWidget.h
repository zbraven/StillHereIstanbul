#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/SHIInventoryComponent.h"
#include "UI/SHIInventorySlotWidget.h"
#include "SHIInventoryWidget.generated.h"

UCLASS()
class STILLHEREISTANBUL_API USHIInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // Widget References
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;

    // Slot widget class
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<USHIInventorySlotWidget> SlotWidgetClass;

    // Slot widgets array
    UPROPERTY()
    TArray<USHIInventorySlotWidget*> SlotWidgets;

    // Character reference
    UPROPERTY()
    class ASHICharacter* OwnerCharacter;

    // Inventory component reference
    UPROPERTY()
    USHIInventoryComponent* InventoryComponent;

public:
    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void SetOwnerCharacter(class ASHICharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void InitializeInventoryGrid();

    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void RefreshInventoryDisplay();

    // Event handlers
    UFUNCTION()
    void OnInventoryChanged(int32 SlotIndex, const FSHIInventorySlot& NewSlot);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Inventory UI")
    int32 GetTotalSlots() const { return TotalSlots; }

    UFUNCTION(BlueprintPure, Category = "Inventory UI")
    int32 GetUsedSlots() const;

    UFUNCTION(BlueprintPure, Category = "Inventory UI")
    USHIInventorySlotWidget* GetSlotWidget(int32 SlotIndex) const;

protected:
    // Grid setup
    void CreateSlotWidgets();
    void UpdateSlotWidget(int32 SlotIndex, const FSHIInventorySlot& SlotData);

    // Constants
    static constexpr int32 GridColumns = 8;
    static constexpr int32 GridRows = 4;
    static constexpr int32 TotalSlots = 30;
};