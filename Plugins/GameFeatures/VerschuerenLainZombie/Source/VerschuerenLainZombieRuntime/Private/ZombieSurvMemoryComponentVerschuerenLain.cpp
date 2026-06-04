#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Common/InventoryComponent.h"
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
	float MinDist = MAX_FLT;
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

void UZombieSurvMemoryComponentVerschuerenLain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	UInventoryComponent* InvComp = PawnOwner->GetComponentByClass<UInventoryComponent>();
	FVector PawnLoc = PawnOwner->GetActorLocation();

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
