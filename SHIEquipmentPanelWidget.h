#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Data/SHIItemData.h"
#include "UI/SHIEquipmentSlotWidget.h"
#include "SHIEquipmentPanelWidget.generated.h"

// Forward declarations
class ASHICharacter;
class USHIEquipmentComponent;
class USHIStatsComponent;

UCLASS()
class STILLHEREISTANBUL_API USHIEquipmentPanelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    USHIEquipmentPanelWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;

    // Owner character reference
    UPROPERTY(BlueprintReadOnly, Category = "SHI Character")
    ASHICharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "SHI Components")
    USHIEquipmentComponent* EquipmentComponent;

    UPROPERTY(BlueprintReadOnly, Category = "SHI Components")
    USHIStatsComponent* StatsComponent;

    // Equipment slot widgets (11 slots total)
    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* KaskSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* GoguslukZirhiSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* KalkanSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* Silah1Slot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* Silah2Slot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* KolyeSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* YuzukSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* KupeSlot;

    // ⬅️ NEW SLOTS:
    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* EldivenSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* PantolonSlot;

    UPROPERTY(meta = (BindWidget))
    USHIEquipmentSlotWidget* AyakkabiSlot;

    // Stats display text widgets
    UPROPERTY(meta = (BindWidget))
    UTextBlock* GucText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CeviklikText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ZekaText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* OdaklanmaText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DayaniklilikText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SaglikText;

public:
    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "SHI Equipment Panel")
    void SetOwnerCharacter(ASHICharacter* Character);

    // UI update functions
    UFUNCTION(BlueprintCallable, Category = "SHI Equipment Panel")
    void RefreshEquipmentDisplay();

    UFUNCTION(BlueprintCallable, Category = "SHI Equipment Panel")
    void RefreshStatsDisplay();

    // Equipment event handlers
    UFUNCTION()
    void OnEquipmentChanged(ESHIEquipmentSlot SlotType, USHIItemData* NewItem, USHIItemData* OldItem);

    UFUNCTION()
    void OnStatsRecalculated();

    // Helper functions
    UFUNCTION(BlueprintPure, Category = "SHI Equipment Panel")
    USHIEquipmentSlotWidget* GetSlotWidget(ESHIEquipmentSlot SlotType);

protected:
    // Internal helper functions
    void BindEquipmentEvents();
    void InitializeSlots();
};