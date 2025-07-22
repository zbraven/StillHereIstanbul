#include "SHIWorldItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/SHICharacter.h"
#include "Components/SHIInventoryComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ASHIWorldItem::ASHIWorldItem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create item mesh
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemMesh->SetCastShadow(false);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(InteractionRadius);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create item name widget
    ItemNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemNameWidget"));
    ItemNameWidget->SetupAttachment(RootComponent);
    ItemNameWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemNameWidget->SetDrawSize(FVector2D(200.0f, 50.0f));
    ItemNameWidget->SetVisibility(false); // Hidden by default

    // Initialize properties
    ItemQuantity = 1;
    bShouldFloat = true;
    FloatAmplitude = 10.0f;
    FloatSpeed = 2.0f;
    FloatTime = 0.0f;
}

void ASHIWorldItem::BeginPlay()
{
    Super::BeginPlay();
    
    // Store starting location for floating animation
    StartLocation = GetActorLocation();
    
    // Setup interaction sphere events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASHIWorldItem::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ASHIWorldItem::OnInteractionSphereEndOverlap);
    
    // Update interaction sphere radius
    InteractionSphere->SetSphereRadius(InteractionRadius);
    
    // Setup item appearance
    if (ItemData)
    {
        // Set mesh if available
        if (ItemData->WorldMesh)
        {
            ItemMesh->SetStaticMesh(ItemData->WorldMesh);
        }
        
        // Apply rarity color
        if (UMaterialInstanceDynamic* DynamicMaterial = ItemMesh->CreateAndSetMaterialInstanceDynamic(0))
        {
            DynamicMaterial->SetVectorParameterValue(FName("ItemColor"), ItemData->RarityColor);
        }
        
        UE_LOG(LogTemp, Log, TEXT("World item initialized: %s x%d"), 
               *ItemData->ItemName.ToString(), ItemQuantity);
    }
}

void ASHIWorldItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Floating animation
    if (bShouldFloat)
    {
        FloatTime += DeltaTime * FloatSpeed;
        float FloatOffset = FMath::Sin(FloatTime) * FloatAmplitude;
        FVector NewLocation = StartLocation + FVector(0, 0, FloatOffset);
        SetActorLocation(NewLocation);
    }
}

void ASHIWorldItem::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ASHICharacter* Character = Cast<ASHICharacter>(OtherActor))
    {
        // Show item name widget
        if (ItemNameWidget)
        {
            ItemNameWidget->SetVisibility(true);
        }
        
        // Show pickup prompt
        if (GEngine && ItemData)
        {
            FString PickupText = FString::Printf(TEXT("E tuşuna basarak %s al"), 
                                               *ItemData->ItemName.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, PickupText);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player entered pickup range for %s"), 
               ItemData ? *ItemData->ItemName.ToString() : TEXT("Unknown Item"));
    }
}

void ASHIWorldItem::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ASHICharacter* Character = Cast<ASHICharacter>(OtherActor))
    {
        // Hide item name widget
        if (ItemNameWidget)
        {
            ItemNameWidget->SetVisibility(false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player left pickup range for %s"), 
               ItemData ? *ItemData->ItemName.ToString() : TEXT("Unknown Item"));
    }
}

bool ASHIWorldItem::TryPickupItem(ASHICharacter* Character)
{
    if (!Character || !ItemData)
    {
        return false;
    }
    
    USHIInventoryComponent* InventoryComponent = Character->GetInventoryComponent();
    if (!InventoryComponent)
    {
        return false;
    }
    
    // Check if inventory can hold the item
    if (InventoryComponent->CanAddItem(ItemData, ItemQuantity))
    {
        // Add item to inventory
        InventoryComponent->Server_AddItem(ItemData, ItemQuantity);
        
        UE_LOG(LogTemp, Log, TEXT("Player picked up %s x%d"), 
               *ItemData->ItemName.ToString(), ItemQuantity);
        
        // Show pickup feedback
        if (GEngine)
        {
            FString PickupText = FString::Printf(TEXT("Alındı: %s x%d"), 
                                               *ItemData->ItemName.ToString(), ItemQuantity);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, PickupText);
        }
        
        // Destroy the world item
        Destroy();
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot pickup %s - inventory full"), 
               *ItemData->ItemName.ToString());
        
        // Show error feedback
        if (GEngine)
        {
            FString ErrorText = FString::Printf(TEXT("Envanter dolu: %s"), 
                                              *ItemData->ItemName.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, ErrorText);
        }
        
        return false;
    }
}

void ASHIWorldItem::InitializeWorldItem(USHIItemData* InItemData, int32 InQuantity)
{
    ItemData = InItemData;
    ItemQuantity = InQuantity;
    
    if (ItemData)
    {
        // Set mesh if available
        if (ItemData->WorldMesh && ItemMesh)
        {
            ItemMesh->SetStaticMesh(ItemData->WorldMesh);
        }
        
        UE_LOG(LogTemp, Log, TEXT("World item initialized: %s x%d"), 
               *ItemData->ItemName.ToString(), ItemQuantity);
    }
}

FText ASHIWorldItem::GetItemDisplayName() const
{
    if (ItemData)
    {
        if (ItemQuantity > 1)
        {
            return FText::FromString(FString::Printf(TEXT("%s x%d"), 
                                                   *ItemData->ItemName.ToString(), ItemQuantity));
        }
        else
        {
            return ItemData->ItemName;
        }
    }
    return FText::FromString(TEXT("Unknown Item"));
}