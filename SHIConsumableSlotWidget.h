#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/SHIItemData.h"
#include "SHIConsumableSlotWidget.generated.h"

class ASHICharacter;
class UImage;
class UTextBlock;
class UButton;

UCLASS()
class STILLHEREISTANBUL_API USHIConsumableSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USHIConsumableSlotWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	// UI Elements
	UPROPERTY(meta = (BindWidget))
	UButton* SlotButton;

	UPROPERTY(meta = (BindWidget))
	UImage* SlotBackground;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuantityText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeyText; // Shows which key (3,4,5,6)

public:
	// Setup functions
	UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
	void SetSlotIndex(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
	void SetOwnerCharacter(ASHICharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
	void SetItem(USHIItemData* Item, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "SHI Consumables")
	void UseSlot();

protected:
	// Properties
	UPROPERTY()
	int32 SlotIndex; // 3, 4, 5, or 6

	UPROPERTY()
	ASHICharacter* OwnerCharacter;

	UPROPERTY()
	USHIItemData* CurrentItem;

	UPROPERTY()
	int32 CurrentQuantity;

	// Button events
	UFUNCTION()
	void OnSlotClicked();

	UFUNCTION()
	void OnSlotHovered();

	UFUNCTION()
	void OnSlotUnhovered();

private:
	// Visual state management
	void UpdateVisualState();
	void SetSlotEmpty();
	void SetSlotFilled();
};