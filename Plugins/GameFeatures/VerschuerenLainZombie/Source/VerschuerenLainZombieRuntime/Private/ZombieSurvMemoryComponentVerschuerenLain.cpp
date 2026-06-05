#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Zombies/BaseZombie.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "GameFramework/Pawn.h"

UZombieSurvMemoryComponentVerschuerenLain::UZombieSurvMemoryComponentVerschuerenLain()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UZombieSurvMemoryComponentVerschuerenLain::AddOrUpdateItem(ABaseItem* Item)
{
	if (!Item) return;

	// check if already in list
	for (auto& Known : KnownItems)
	{
		if (Known.Item == Item)
		{
			Known.Location = Item->GetActorLocation();
			Known.Value = Item->GetValue();
			return;
		}
	}

	// add new memory entry
	FMemoryItemVerschuerenLain NewItem;
	NewItem.Item = Item;
	NewItem.Location = Item->GetActorLocation();
	NewItem.ItemType = Item->GetItemType();
	NewItem.Value = Item->GetValue();
	KnownItems.Add(NewItem);
}

void UZombieSurvMemoryComponentVerschuerenLain::RemoveItem(ABaseItem* Item)
{
	for (int i = KnownItems.Num() - 1; i >= 0; --i)
	{
		if (KnownItems[i].Item == Item)
		{
			KnownItems.RemoveAt(i);
		}
	}
}

bool UZombieSurvMemoryComponentVerschuerenLain::FindClosestItem(const FVector& Origin, EItemType Type, FVector& OutLocation, ABaseItem*& OutItem)
{
	float MinDist = FLT_MAX;
	bool bFound = false;

	for (const auto& Known : KnownItems)
	{
		if (!IsValid(Known.Item) || Known.Item->IsHidden()) continue;

		// garbage means wildcard
		if (Type != EItemType::Garbage && Known.ItemType != Type) continue;

		float Dist = FVector::Dist(Origin, Known.Location);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			OutLocation = Known.Location;
			OutItem = Known.Item;
			bFound = true;
		}
	}

	return bFound;
}

void UZombieSurvMemoryComponentVerschuerenLain::AddZombie(ABaseZombie* Zombie)
{
	if (Zombie && !KnownZombies.Contains(Zombie))
	{
		KnownZombies.Add(Zombie);
	}
}

void UZombieSurvMemoryComponentVerschuerenLain::RemoveZombie(ABaseZombie* Zombie)
{
	KnownZombies.Remove(Zombie);
}

bool UZombieSurvMemoryComponentVerschuerenLain::FindClosestZombie(const FVector& Origin, ABaseZombie*& OutZombie)
{
	float MinDist = FLT_MAX;
	bool bFound = false;

	for (ABaseZombie* Zombie : KnownZombies)
	{
		if (!IsValid(Zombie)) continue;

		float Dist = FVector::Dist(Origin, Zombie->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			OutZombie = Zombie;
			bFound = true;
		}
	}

	return bFound;
}

void UZombieSurvMemoryComponentVerschuerenLain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	UInventoryComponent* InvComp = PawnOwner->GetComponentByClass<UInventoryComponent>();
	FVector PawnLoc = PawnOwner->GetActorLocation();

	// clean up dead zombie references
	for (int i = KnownZombies.Num() - 1; i >= 0; --i)
	{
		if (!IsValid(KnownZombies[i]))
		{
			KnownZombies.RemoveAt(i);
		}
	}

	// grab nearby known items
	for (int i = KnownItems.Num() - 1; i >= 0; --i)
	{
		auto& Known = KnownItems[i];

		if (!IsValid(Known.Item) || Known.Item->IsHidden())
		{
			KnownItems.RemoveAt(i);
			continue;
		}

		if (InvComp)
		{
			float Dist = FVector::Dist(PawnLoc, Known.Item->GetActorLocation());
			if (Dist <= InvComp->GetPickupRange())
			{
				// find first empty slot
				int EmptySlot = -1;
				const TArray<ABaseItem*>& Inv = InvComp->GetInventory();
				for (int s = 0; s < Inv.Num(); ++s)
				{
					if (Inv[s] == nullptr)
					{
						EmptySlot = s;
						break;
					}
				}

				// if inventory is full, try to use food/medkit to free slot
				if (EmptySlot == -1)
				{
					UHealthComponent* HealthComp = PawnOwner->GetComponentByClass<UHealthComponent>();
					UStaminaComponent* StaminaComp = PawnOwner->GetComponentByClass<UStaminaComponent>();

					int MedkitSlot = -1;
					int FoodSlot = -1;
					for (int s = 0; s < Inv.Num(); ++s)
					{
						if (Inv[s])
						{
							if (Inv[s]->GetItemType() == EItemType::Medkit) MedkitSlot = s;
							else if (Inv[s]->GetItemType() == EItemType::Food) FoodSlot = s;
						}
					}

					// 1. health is low, use medkit
					if (HealthComp && HealthComp->GetHealth() < HealthComp->GetMaxHealth() && MedkitSlot != -1)
					{
						InvComp->UseItem(MedkitSlot);
						InvComp->RemoveItem(MedkitSlot);
						EmptySlot = MedkitSlot;
					}
					// 2. stamina is low, use food
					else if (StaminaComp && StaminaComp->GetCurrentStamina() < StaminaComp->GetMaxStamina() && FoodSlot != -1)
					{
						InvComp->UseItem(FoodSlot);
						InvComp->RemoveItem(FoodSlot);
						EmptySlot = FoodSlot;
					}
					// 3. still full, consume food anyway to free space
					else if (FoodSlot != -1)
					{
						InvComp->UseItem(FoodSlot);
						InvComp->RemoveItem(FoodSlot);
						EmptySlot = FoodSlot;
					}
					// 4. consume medkit anyway
					else if (MedkitSlot != -1)
					{
						InvComp->UseItem(MedkitSlot);
						InvComp->RemoveItem(MedkitSlot);
						EmptySlot = MedkitSlot;
					}
				}

				if (EmptySlot != -1)
				{
					if (InvComp->GrabItem(EmptySlot, Known.Item))
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, 
							FString::Printf(TEXT("grabbed item at %s"), *Known.Location.ToString()));
						KnownItems.RemoveAt(i);
					}
				}
			}
		}
	}
}
