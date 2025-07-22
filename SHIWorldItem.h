#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/SHIItemData.h"
#include "SHIWorldItem.generated.h"

UCLASS()
class STILLHEREISTANBUL_API ASHIWorldItem : public AActor
{
    GENERATED_BODY()
    
public:    
    ASHIWorldItem();

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ItemMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UWidgetComponent* ItemNameWidget;

    // Item Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
    USHIItemData* ItemData;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
    int32 ItemQuantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
    float InteractionRadius = 150.0f;

    // Visual Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Properties")
    bool bShouldFloat = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Properties")
    float FloatAmplitude = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual Properties")
    float FloatSpeed = 2.0f;
    
    UPROPERTY()
    float FloatTime = 0.0f;
    
    UPROPERTY()
    FVector StartLocation;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Interaction events
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // Pickup function
    UFUNCTION(BlueprintCallable, Category = "Item Pickup")
    bool TryPickupItem(class ASHICharacter* Character);
    
    // Setup function
    UFUNCTION(BlueprintCallable, Category = "Item Setup")
    void InitializeWorldItem(USHIItemData* InItemData, int32 InQuantity = 1);
    
    // Getters
    UFUNCTION(BlueprintPure, Category = "Item Info")
    USHIItemData* GetItemData() const { return ItemData; }
    
    UFUNCTION(BlueprintPure, Category = "Item Info")
    int32 GetItemQuantity() const { return ItemQuantity; }
    
    UFUNCTION(BlueprintPure, Category = "Item Info")
    FText GetItemDisplayName() const;
};