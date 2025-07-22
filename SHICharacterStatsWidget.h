#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
// #include "Player/SHICharacter.h"  // BU SATIRI KALDIR
#include "SHICharacterStatsWidget.generated.h"

class ASHICharacter;  // Forward declaration ekle

UCLASS()
class STILLHEREISTANBUL_API USHICharacterStatsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Text Block References (Bind these in Blueprint)
    UPROPERTY(meta = (BindWidget))
    UTextBlock* GucValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CeviklikValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ZekaValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* OdaklanmaValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DayaniklilikValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SaglikValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* EnerjiValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* HasarValue;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SavunmaValue;

    // Character reference
    UPROPERTY()
    ASHICharacter* OwnerCharacter;

public:
    // Update functions (can be called from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Stats Update")
    void UpdateAllStats();
    
    UFUNCTION(BlueprintCallable, Category = "Stats Update")
    void SetOwnerCharacter(ASHICharacter* Character);

    // Individual stat getters (for Blueprint binding)
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetGucText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetCeviklikText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetZekaText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetOdaklanmaText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetDayaniklilikText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetSaglikText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetEnerjiText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetHasarText() const;
    
    UFUNCTION(BlueprintPure, Category = "Stats Display")
    FText GetSavunmaText() const;
};