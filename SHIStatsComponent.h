#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "SHIStatsComponent.generated.h"

// Forward declaration
struct FSHIStatModifier;

// UE5.6 Enhanced Stat Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatChanged, FName, StatName, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatsRecalculated);

// Turkish RPG stat structure
USTRUCT(BlueprintType)
struct FSHICharacterStats : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Stats")
    float Guc = 10.0f; // Güç (Strength)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Stats")
    float Ceviklik = 10.0f; // Çeviklik (Dexterity)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Stats")
    float Zeka = 10.0f; // Zeka (Intelligence)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Stats")
    float Odaklanma = 10.0f; // Odaklanma (Focus)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Stats")
    float Dayaniklilik = 10.0f; // Dayanıklılık (Constitution)

    FSHICharacterStats()
    {
        Guc = 10.0f;
        Ceviklik = 10.0f;
        Zeka = 10.0f;
        Odaklanma = 10.0f;
        Dayaniklilik = 10.0f;
    }

    // Operators for stat calculations
    FSHICharacterStats operator+(const FSHICharacterStats& Other) const
    {
        FSHICharacterStats Result;
        Result.Guc = Guc + Other.Guc;
        Result.Ceviklik = Ceviklik + Other.Ceviklik;
        Result.Zeka = Zeka + Other.Zeka;
        Result.Odaklanma = Odaklanma + Other.Odaklanma;
        Result.Dayaniklilik = Dayaniklilik + Other.Dayaniklilik;
        return Result;
    }

    // Apply modifiers from equipment
    void ApplyModifiers(const TArray<FSHIStatModifier>& Modifiers);
};

UCLASS(ClassGroup=(SHI), meta=(BlueprintSpawnableComponent))
class STILLHEREISTANBUL_API USHIStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USHIStatsComponent();

protected:
    // UE5.6 Enhanced Replication - Base stats (permanent character progression)
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character Stats")
    FSHICharacterStats BaseStats;

    // Current stats (base + equipment + temporary modifiers)
    UPROPERTY(ReplicatedUsing = OnRep_CurrentStats, BlueprintReadOnly, Category = "Character Stats")
    FSHICharacterStats CurrentStats;

    // Equipment bonuses (calculated from equipment component)
    UPROPERTY(BlueprintReadOnly, Category = "Character Stats")
    FSHICharacterStats EquipmentBonuses;

    // Temporary modifiers (buffs, debuffs, etc)
    UPROPERTY(BlueprintReadOnly, Category = "Character Stats")
    FSHICharacterStats TemporaryModifiers;

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // Base stat modification (permanent character progression)
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Stats")
    void Server_ModifyBaseStat(FName StatName, float Amount);

    // Equipment bonus system
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ApplyEquipmentBonuses(const TArray<FSHIStatModifier>& EquipmentModifiers);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ClearEquipmentBonuses();

    // Temporary modifier system (for buffs/debuffs)
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ApplyTemporaryModifier(FName StatName, float Amount, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RemoveTemporaryModifier(FName StatName);

    // Stat access functions (current stats = base + equipment + temporary)
    UFUNCTION(BlueprintPure, Category = "Stats Access")
    FSHICharacterStats GetBaseStats() const { return BaseStats; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    FSHICharacterStats GetCurrentStats() const { return CurrentStats; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    FSHICharacterStats GetEquipmentBonuses() const { return EquipmentBonuses; }

    // Individual current stat getters (for easier access)
    UFUNCTION(BlueprintPure, Category = "Stats Access")
    float GetCurrentGuc() const { return CurrentStats.Guc; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    float GetCurrentZeka() const { return CurrentStats.Zeka; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    float GetCurrentCeviklik() const { return CurrentStats.Ceviklik; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    float GetCurrentOdaklanma() const { return CurrentStats.Odaklanma; }

    UFUNCTION(BlueprintPure, Category = "Stats Access")
    float GetCurrentDayaniklilik() const { return CurrentStats.Dayaniklilik; }

    // Derived stats (Istanbul-themed calculations)
    UFUNCTION(BlueprintPure, Category = "Derived Stats")
    float GetMaxSaglik() const; // Max Sağlık (Health)

    UFUNCTION(BlueprintPure, Category = "Derived Stats")
    float GetMaxEnerji() const; // Max Enerji (Mana)

    UFUNCTION(BlueprintPure, Category = "Derived Stats")
    float GetHasarBonusu() const; // Hasar Bonusu (Damage Bonus)

    UFUNCTION(BlueprintPure, Category = "Derived Stats")
    float GetSavunma() const; // Savunma (Defense)

    // Stat threshold system (for attribute bonuses at 50, 100, 150, etc.)
    UFUNCTION(BlueprintPure, Category = "Threshold System")
    int32 GetStatThresholdLevel(FName StatName) const;

    UFUNCTION(BlueprintPure, Category = "Threshold System")
    TArray<FText> GetActiveThresholdBonuses() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStatsRecalculated OnStatsRecalculated;

protected:
    // Network replication
    UFUNCTION()
    void OnRep_CurrentStats();

    // Internal stat calculation
    void RecalculateCurrentStats();

    // Helper functions
    float GetStatByName(const FSHICharacterStats& Stats, FName StatName) const;
    void SetStatByName(FSHICharacterStats& Stats, FName StatName, float Value);

    // Temporary modifiers with timers
    TMap<FName, FTimerHandle> TempModifierTimers;
    TMap<FName, float> TempModifierValues;

public:
    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Stats Debug")
    void DebugPrintStats() const;

    UFUNCTION(BlueprintCallable, Category = "Stats Debug")
    FString GetStatsDebugString() const;
};