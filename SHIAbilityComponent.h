// SHIAbilityComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/SHIItemData.h"
#include "Net/UnrealNetwork.h"
#include "SHIAbilityComponent.generated.h"

class ASHICharacter;
class USHIItemData;

// Note: ESHIWeaponAbilityType enum moved to SHIItemData.h to avoid circular dependency

// Ability data structure
USTRUCT(BlueprintType)
struct FSHIAbilityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    ESHIWeaponAbilityType AbilityType = ESHIWeaponAbilityType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    FText AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    FText AbilityDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float CooldownTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    float Range = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    TSoftObjectPtr<UTexture2D> AbilityIcon;

    FSHIAbilityData()
    {
        AbilityType = ESHIWeaponAbilityType::None;
        AbilityName = FText::FromString("Unknown Ability");
        AbilityDescription = FText::FromString("No description");
        CooldownTime = 5.0f;
        BaseDamage = 50.0f;
        Range = 300.0f;
    }
};

// Active ability state for cooldowns
USTRUCT(BlueprintType)
struct FSHIActiveAbility
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    ESHIWeaponAbilityType AbilityType = ESHIWeaponAbilityType::None;

    UPROPERTY(BlueprintReadOnly)
    float RemainingCooldown = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsOnCooldown = false;

    FSHIActiveAbility()
    {
        AbilityType = ESHIWeaponAbilityType::None;
        RemainingCooldown = 0.0f;
        bIsOnCooldown = false;
    }
};

// Replicated ability state for networking
USTRUCT(BlueprintType)
struct FSHIAbilityState
{
    GENERATED_BODY()

    // Current weapon abilities (Q, R, F slots)
    UPROPERTY(BlueprintReadOnly)
    FSHIAbilityData QAbility;

    UPROPERTY(BlueprintReadOnly)
    FSHIAbilityData RAbility;

    UPROPERTY(BlueprintReadOnly)
    FSHIAbilityData FAbility;

    // Active cooldowns
    UPROPERTY(BlueprintReadOnly)
    FSHIActiveAbility QAbilityCooldown;

    UPROPERTY(BlueprintReadOnly)
    FSHIActiveAbility RAbilityCooldown;

    UPROPERTY(BlueprintReadOnly)
    FSHIActiveAbility FAbilityCooldown;

    FSHIAbilityState()
    {
        QAbility = FSHIAbilityData();
        RAbility = FSHIAbilityData();
        FAbility = FSHIAbilityData();
        QAbilityCooldown = FSHIActiveAbility();
        RAbilityCooldown = FSHIActiveAbility();
        FAbilityCooldown = FSHIActiveAbility();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityUsed, ESHIWeaponAbilityType, AbilityType, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitiesChanged, const FSHIAbilityState&, NewAbilityState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCooldownUpdated, ESHIWeaponAbilityType, AbilityType, float, RemainingTime);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STILLHEREISTANBUL_API USHIAbilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USHIAbilityComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Replicated ability state
    UPROPERTY(ReplicatedUsing = OnRep_AbilityState)
    FSHIAbilityState AbilityState;

    // Character reference
    UPROPERTY()
    ASHICharacter* OwnerCharacter = nullptr;

public:
    // Event dispatchers
    UPROPERTY(BlueprintAssignable, Category = "SHI Abilities")
    FOnAbilityUsed OnAbilityUsed;

    UPROPERTY(BlueprintAssignable, Category = "SHI Abilities")
    FOnAbilitiesChanged OnAbilitiesChanged;

    UPROPERTY(BlueprintAssignable, Category = "SHI Abilities")
    FOnAbilityCooldownUpdated OnAbilityCooldownUpdated;

    // Main ability functions
    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseQAbility();

    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseRAbility();

    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UseFAbility();

    // Server functions
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "SHI Network")
    void Server_UseAbility(ESHIWeaponAbilityType AbilityType);

    // Update abilities based on active weapon
    UFUNCTION(BlueprintCallable, Category = "SHI Abilities")
    void UpdateAbilitiesForWeapon(USHIItemData* WeaponData);

    // Getters
    UFUNCTION(BlueprintPure, Category = "SHI Abilities")
    FSHIAbilityState GetCurrentAbilityState() const { return AbilityState; }

    UFUNCTION(BlueprintPure, Category = "SHI Abilities")
    bool IsAbilityOnCooldown(ESHIWeaponAbilityType AbilityType) const;

    UFUNCTION(BlueprintPure, Category = "SHI Abilities")
    float GetAbilityCooldownRemaining(ESHIWeaponAbilityType AbilityType) const;

    UFUNCTION(BlueprintPure, Category = "SHI Abilities")
    bool CanUseAbility(ESHIWeaponAbilityType AbilityType) const;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "SHI Debug")
    void DebugPrintAbilities() const;

protected:
    // Replication
    UFUNCTION()
    void OnRep_AbilityState();

    // Internal ability logic
    void UseAbilityInternal(ESHIWeaponAbilityType AbilityType);
    void StartAbilityCooldown(ESHIWeaponAbilityType AbilityType, float CooldownTime);
    void UpdateCooldowns(float DeltaTime);

    // Weapon-specific ability mappings
    TArray<FSHIAbilityData> GetAbilitiesForWeaponType(const FString& WeaponTypeName) const;
    TArray<FSHIAbilityData> GetAbilitiesFromWeaponData(USHIItemData* WeaponData) const;
    FSHIAbilityData CreateAbilityData(ESHIWeaponAbilityType AbilityType) const;

    // Turkish-themed ability definitions
    void InitializeAbilityDatabase();
    TMap<ESHIWeaponAbilityType, FSHIAbilityData> AbilityDatabase;
};