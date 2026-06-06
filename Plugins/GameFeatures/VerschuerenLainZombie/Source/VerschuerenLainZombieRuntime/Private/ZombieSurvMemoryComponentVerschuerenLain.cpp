#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"
#include "PurgeZones/PurgeZone.h"
#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "GameFramework/Pawn.h"
#include "Survivor/SurvivorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"

UZombieSurvMemoryComponentVerschuerenLain::UZombieSurvMemoryComponentVerschuerenLain()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UZombieSurvMemoryComponentVerschuerenLain::BeginPlay()
{
	Super::BeginPlay();

	// find all houses
	TArray<AActor*> FoundHouses;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHouse::StaticClass(), FoundHouses);
	for (AActor* Actor : FoundHouses)
	{
		AHouse* House = Cast<AHouse>(Actor);
		if (House)
		{
			FHouseMemoryVerschuerenLain Entry;
			Entry.House = House;
			Entry.bExplored = false;
			Entry.EntryPoint = House->GetBounds().Origin;
			HousesMemory.Add(Entry);
		}
	}
}

void UZombieSurvMemoryComponentVerschuerenLain::AddOrUpdateItem(ABaseItem* Item)
{
	if (!Item) return;
	
	// do not pick up garbage
	if (Item->GetItemType() == EItemType::Garbage) return;

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

		// skip items inside purge zones
		if (IsLocationInPurgeZone(Known.Location, 200.f)) continue;

		// skip items if the path to them intersects any purge zone
		if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(GetOwner()))
		{
			TArray<FVector> Path = Survivor->CalculatePath(Known.Location);
			if (DoesPathIntersectPurgeZone(Path, 150.f)) continue;
		}

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
	if (!Zombie) return;

	for (auto& MemZombie : KnownZombies)
	{
		if (MemZombie.Zombie == Zombie)
		{
			MemZombie.LastKnownLocation = Zombie->GetActorLocation();
			MemZombie.bSensed = true;
			MemZombie.LastTimeSeen = GetWorld()->GetTimeSeconds();
			return;
		}
	}

	FMemoryZombieVerschuerenLain NewEntry;
	NewEntry.Zombie = Zombie;
	NewEntry.LastKnownLocation = Zombie->GetActorLocation();
	NewEntry.bSensed = true;
	NewEntry.LastTimeSeen = GetWorld()->GetTimeSeconds();
	KnownZombies.Add(NewEntry);
}

void UZombieSurvMemoryComponentVerschuerenLain::RemoveZombie(ABaseZombie* Zombie)
{
	if (!Zombie) return;

	for (auto& MemZombie : KnownZombies)
	{
		if (MemZombie.Zombie == Zombie)
		{
			MemZombie.bSensed = false;
			MemZombie.LastTimeSeen = GetWorld()->GetTimeSeconds();
			return;
		}
	}
}

bool UZombieSurvMemoryComponentVerschuerenLain::FindClosestZombie(const FVector& Origin, ABaseZombie*& OutZombie)
{
	float MinDist = FLT_MAX;
	bool bFound = false;

	for (const auto& MemZombie : KnownZombies)
	{
		if (!IsValid(MemZombie.Zombie) || IsZombieDead(MemZombie.Zombie)) continue;

		float Dist = FVector::Dist(Origin, MemZombie.Zombie->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			OutZombie = MemZombie.Zombie;
			bFound = true;
		}
	}

	return bFound;
}

bool UZombieSurvMemoryComponentVerschuerenLain::GetNextUnexploredHouse(const FVector& Origin, FVector& OutLocation, AHouse*& OutHouse)
{
	float MinDist = FLT_MAX;
	bool bFound = false;

	for (auto& HouseMem : HousesMemory)
	{
		if (!HouseMem.bExplored && IsValid(HouseMem.House))
		{
			// skip houses inside purge zones
			if (IsLocationInPurgeZone(HouseMem.EntryPoint, 200.f)) continue;

			// skip houses if the path to them intersects any purge zone
			if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(GetOwner()))
			{
				TArray<FVector> Path = Survivor->CalculatePath(HouseMem.EntryPoint);
				if (DoesPathIntersectPurgeZone(Path, 150.f)) continue;
			}

			float Dist = FVector::Dist(Origin, HouseMem.EntryPoint);
			if (Dist < MinDist)
			{
				MinDist = Dist;
				OutLocation = HouseMem.EntryPoint;
				OutHouse = HouseMem.House;
				bFound = true;
			}
		}
	}
	return bFound;
}

void UZombieSurvMemoryComponentVerschuerenLain::MarkHouseExplored(AHouse* House)
{
	for (auto& HouseMem : HousesMemory)
	{
		if (HouseMem.House == House)
		{
			HouseMem.bExplored = true;
			break;
		}
	}
}

void UZombieSurvMemoryComponentVerschuerenLain::MarkClosestHouseExplored(const FVector& Origin, float Radius)
{
	for (auto& HouseMem : HousesMemory)
	{
		if (IsValid(HouseMem.House))
		{
			float Dist = FVector::Dist(Origin, HouseMem.EntryPoint);
			if (Dist <= Radius)
			{
				HouseMem.bExplored = true;
				break;
			}
		}
	}
}

bool UZombieSurvMemoryComponentVerschuerenLain::IsZombieDead(ABaseZombie* Zombie)
{
	if (!IsValid(Zombie)) return true;
	if (UHealthComponent* HealthComp = Zombie->GetComponentByClass<UHealthComponent>())
	{
		return HealthComp->IsDead();
	}
	return false;
}

bool UZombieSurvMemoryComponentVerschuerenLain::IsLocationInPurgeZone(const FVector& Location, float SafetyMargin)
{
	TArray<AActor*> FoundPurgeZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APurgeZone::StaticClass(), FoundPurgeZones);

	for (AActor* ZoneActor : FoundPurgeZones)
	{
		if (IsValid(ZoneActor))
		{
			float Radius = 50.f;
			FProperty* Prop = ZoneActor->GetClass()->FindPropertyByName(TEXT("Diameter"));
			if (Prop)
			{
				if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
				{
					Radius = FloatProp->GetPropertyValue_InContainer(ZoneActor) / 2.f;
				}
			}

			float Dist2D = FVector::Dist2D(Location, ZoneActor->GetActorLocation());
			if (Dist2D <= (Radius + SafetyMargin))
			{
				return true;
			}
		}
	}
	return false;
}

bool UZombieSurvMemoryComponentVerschuerenLain::DoesPathIntersectPurgeZone(const TArray<FVector>& Path, float SafetyMargin)
{
	if (Path.Num() == 0) return false;

	// Check if the destination itself is inside/near a purge zone
	if (IsLocationInPurgeZone(Path.Last(), SafetyMargin))
	{
		return true;
	}

	TArray<AActor*> FoundPurgeZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APurgeZone::StaticClass(), FoundPurgeZones);

	for (AActor* ZoneActor : FoundPurgeZones)
	{
		if (!IsValid(ZoneActor)) continue;

		float Radius = 50.f;
		FProperty* Prop = ZoneActor->GetClass()->FindPropertyByName(TEXT("Diameter"));
		if (Prop)
		{
			if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
			{
				Radius = FloatProp->GetPropertyValue_InContainer(ZoneActor) / 2.f;
			}
		}

		FVector ZoneLocation = ZoneActor->GetActorLocation();

		// Check each 2D segment of the path for intersection with the purge cylinder
		for (int i = 0; i < Path.Num() - 1; ++i)
		{
			FVector SegmentStart = Path[i];
			FVector SegmentEnd = Path[i + 1];

			FVector SegmentStart2D(SegmentStart.X, SegmentStart.Y, 0.f);
			FVector SegmentEnd2D(SegmentEnd.X, SegmentEnd.Y, 0.f);
			FVector ZoneLoc2D(ZoneLocation.X, ZoneLocation.Y, 0.f);

			FVector ClosestPoint2D = FMath::ClosestPointOnSegment(ZoneLoc2D, SegmentStart2D, SegmentEnd2D);
			float Dist2D = FVector::Dist(ClosestPoint2D, ZoneLoc2D);

			if (Dist2D <= (Radius + SafetyMargin))
			{
				return true;
			}
		}
	}

	return false;
}

void UZombieSurvMemoryComponentVerschuerenLain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	UInventoryComponent* InvComp = PawnOwner->GetComponentByClass<UInventoryComponent>();
	FVector PawnLoc = PawnOwner->GetActorLocation();

	// Smooth out movement components at runtime
	if (UPawnMovementComponent* MoveComp = PawnOwner->GetMovementComponent())
	{
		if (UFloatingPawnMovement* FloatMoveComp = Cast<UFloatingPawnMovement>(MoveComp))
		{
			FloatMoveComp->Acceleration = 1200.f;
			FloatMoveComp->Deceleration = 1200.f;
		}
	}
	// scan for purge zones
	TArray<AActor*> FoundPurgeZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APurgeZone::StaticClass(), FoundPurgeZones);

	bool bInPurgeZone = false;
	FVector EscapeLocation = FVector::ZeroVector;

	// 1. Check if we are physically inside or very close to any purge zone (2D check)
	for (AActor* ZoneActor : FoundPurgeZones)
	{
		if (IsValid(ZoneActor))
		{
			float Radius = 50.f;
			FProperty* Prop = ZoneActor->GetClass()->FindPropertyByName(TEXT("Diameter"));
			if (Prop)
			{
				if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
				{
					Radius = FloatProp->GetPropertyValue_InContainer(ZoneActor) / 2.f;
				}
			}

			float Dist2D = FVector::Dist2D(PawnLoc, ZoneActor->GetActorLocation());
			// if inside zone or very close (with 600 unit safety margin)
			if (Dist2D <= (Radius + 600.f))
			{
				bInPurgeZone = true;
				FVector DirFromCenter = (PawnLoc - ZoneActor->GetActorLocation()).GetSafeNormal2D();
				if (DirFromCenter.IsNearlyZero())
				{
					DirFromCenter = PawnOwner->GetActorForwardVector();
				}
				EscapeLocation = ZoneActor->GetActorLocation() + DirFromCenter * (Radius + 800.f);

				// Safety check: Make sure EscapeLocation itself doesn't lie inside another purge zone.
				if (IsLocationInPurgeZone(EscapeLocation, 200.f))
				{
					for (int angle = 45; angle < 360; angle += 45)
					{
						FVector RotatedDir = DirFromCenter.RotateAngleAxis(angle, FVector::UpVector);
						FVector CandidateLocation = ZoneActor->GetActorLocation() + RotatedDir * (Radius + 800.f);
						if (!IsLocationInPurgeZone(CandidateLocation, 200.f))
						{
							EscapeLocation = CandidateLocation;
							break;
						}
					}
				}
				break; // escape this zone first
			}
		}
	}

	// 2. If not physically in a zone, check if our current path intersects a purge zone
	if (!bInPurgeZone)
	{
		if (auto Controller = PawnOwner->GetController<AAIController>())
		{
			if (auto BlackboardComp = Controller->GetBlackboardComponent())
			{
				FVector TargetLocation = BlackboardComp->GetValueAsVector(TEXT("TargetLocation"));
				if (!TargetLocation.IsNearlyZero() && FVector::Dist2D(PawnLoc, TargetLocation) > 100.f)
				{
					ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(PawnOwner);
					if (Survivor)
					{
						TArray<FVector> Path = Survivor->CalculatePath(TargetLocation);
						if (DoesPathIntersectPurgeZone(Path, 150.f))
						{
							// Find the offending zone to escape from it
							for (AActor* ZoneActor : FoundPurgeZones)
							{
								if (!IsValid(ZoneActor)) continue;

								float Radius = 50.f;
								FProperty* Prop = ZoneActor->GetClass()->FindPropertyByName(TEXT("Diameter"));
								if (Prop)
								{
									if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
									{
										Radius = FloatProp->GetPropertyValue_InContainer(ZoneActor) / 2.f;
									}
								}

								// Check if this specific zone intersects the path
								for (int i = 0; i < Path.Num() - 1; ++i)
								{
									FVector SegmentStart = Path[i];
									FVector SegmentEnd = Path[i + 1];
									FVector SegmentStart2D(SegmentStart.X, SegmentStart.Y, 0.f);
									FVector SegmentEnd2D(SegmentEnd.X, SegmentEnd.Y, 0.f);
									FVector ZoneLoc2D(ZoneActor->GetActorLocation().X, ZoneActor->GetActorLocation().Y, 0.f);

									FVector ClosestPoint2D = FMath::ClosestPointOnSegment(ZoneLoc2D, SegmentStart2D, SegmentEnd2D);
									float Dist2D = FVector::Dist(ClosestPoint2D, ZoneLoc2D);

									if (Dist2D <= (Radius + 150.f))
									{
										bInPurgeZone = true;
										FVector DirFromCenter = (PawnLoc - ZoneActor->GetActorLocation()).GetSafeNormal2D();
										if (DirFromCenter.IsNearlyZero())
										{
											DirFromCenter = PawnOwner->GetActorForwardVector();
										}
										EscapeLocation = ZoneActor->GetActorLocation() + DirFromCenter * (Radius + 800.f);

										// Ensure the escape location is safe
										if (IsLocationInPurgeZone(EscapeLocation, 200.f))
										{
											for (int angle = 45; angle < 360; angle += 45)
											{
												FVector RotatedDir = DirFromCenter.RotateAngleAxis(angle, FVector::UpVector);
												FVector CandidateLocation = ZoneActor->GetActorLocation() + RotatedDir * (Radius + 800.f);
												if (!IsLocationInPurgeZone(CandidateLocation, 200.f))
												{
													EscapeLocation = CandidateLocation;
													break;
												}
											}
										}
										break;
									}
								}
								if (bInPurgeZone) break;
							}
						}
					}
				}
			}
		}
	}

	// update blackboard with purge zone status
	if (auto Controller = PawnOwner->GetController<AAIController>())
	{
		if (auto BlackboardComp = Controller->GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsBool(InPurgeZoneKeyName, bInPurgeZone);
			if (bInPurgeZone)
			{
				BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), EscapeLocation);
				
				// sprint out of the zone
				if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(PawnOwner))
				{
					Survivor->StartRunning();
				}
			}
		}
	}

	UHealthComponent* HealthComp = PawnOwner->GetComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		int CurrentHealth = HealthComp->GetHealth();
		if (LastHealth != -1 && CurrentHealth < LastHealth)
		{
			// only spin if we are NOT inside a purge zone!
			if (!bInPurgeZone)
			{
				ABaseZombie* DummyZombie = nullptr;
				bool bHasZombie = FindClosestZombie(PawnOwner->GetActorLocation(), DummyZombie);
				if (!bHasZombie)
				{
					if (auto Controller = PawnOwner->GetController<AAIController>())
					{
						if (auto BlackboardComp = Controller->GetBlackboardComponent())
						{
							BlackboardComp->SetValueAsBool(Needs360ScanKeyName, true);
						}
					}
				}
			}
		}
		LastHealth = CurrentHealth;
	}

	// periodic scan
	if (PeriodicScanInterval > 0.f)
	{
		ScanTimer += DeltaTime;
		if (ScanTimer >= PeriodicScanInterval)
		{
			ABaseZombie* DummyZombie = nullptr;
			bool bHasZombie = FindClosestZombie(PawnOwner->GetActorLocation(), DummyZombie);
			if (!bHasZombie)
			{
				if (auto Controller = PawnOwner->GetController<AAIController>())
				{
					if (auto BlackboardComp = Controller->GetBlackboardComponent())
					{
						BlackboardComp->SetValueAsBool(Needs360ScanKeyName, true);
					}
				}
			}
			ScanTimer = 0.f;
		}
	}

	// clean up dead or timeout out-of-sight zombie references in memory
	float CurrentTime = GetWorld()->GetTimeSeconds();
	for (int i = KnownZombies.Num() - 1; i >= 0; --i)
	{
		auto& MemZombie = KnownZombies[i];
		if (!IsValid(MemZombie.Zombie) || IsZombieDead(MemZombie.Zombie))
		{
			KnownZombies.RemoveAt(i);
		}
		else if (!MemZombie.bSensed && (CurrentTime - MemZombie.LastTimeSeen) > 3.0f)
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

				// inv full, need to drop
				if (EmptySlot == -1)
				{
					int MedkitCount = 0;
					int FoodCount = 0;
					int WeaponCount = 0;
					int GarbageCount = 0;

					for (int s = 0; s < Inv.Num(); ++s)
					{
						if (Inv[s])
						{
							EItemType Type = Inv[s]->GetItemType();
							if (Type == EItemType::Medkit) MedkitCount++;
							else if (Type == EItemType::Food) FoodCount++;
							else if (Type == EItemType::Pistol || Type == EItemType::Shotgun) WeaponCount++;
							else if (Type == EItemType::Garbage) GarbageCount++;
						}
					}

					int DropSlot = -1;

					// drop garbage
					for (int s = 0; s < Inv.Num(); ++s)
					{
						if (Inv[s] && Inv[s]->GetItemType() == EItemType::Garbage)
						{
							DropSlot = s;
							break;
						}
					}

					// drop extra medkit
					if (DropSlot == -1 && MedkitCount > 1)
					{
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s] && Inv[s]->GetItemType() == EItemType::Medkit)
							{
								DropSlot = s;
								break;
							}
						}
					}

					// drop extra food
					if (DropSlot == -1 && FoodCount > 1)
					{
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s] && Inv[s]->GetItemType() == EItemType::Food)
							{
								DropSlot = s;
								break;
							}
						}
					}

					// drop gun with least ammo
					if (DropSlot == -1 && WeaponCount > 0)
					{
						int MinAmmo = INT_MAX;
						int BestWeaponSlot = -1;
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s])
							{
								EItemType Type = Inv[s]->GetItemType();
								if (Type == EItemType::Pistol || Type == EItemType::Shotgun)
								{
									int Ammo = Inv[s]->GetValue();
									if (Ammo < MinAmmo)
									{
										MinAmmo = Ammo;
										BestWeaponSlot = s;
									}
								}
							}
						}
						DropSlot = BestWeaponSlot;
					}

					// fallback drop medkit
					if (DropSlot == -1)
					{
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s] && Inv[s]->GetItemType() == EItemType::Medkit)
							{
								DropSlot = s;
								break;
							}
						}
					}

					// fallback drop food
					if (DropSlot == -1)
					{
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s] && Inv[s]->GetItemType() == EItemType::Food)
							{
								DropSlot = s;
								break;
							}
						}
					}

					// absolute fallback
					if (DropSlot == -1)
					{
						for (int s = 0; s < Inv.Num(); ++s)
						{
							if (Inv[s])
							{
								DropSlot = s;
								break;
							}
						}
					}

					if (DropSlot != -1)
					{
						InvComp->RemoveItem(DropSlot);
						EmptySlot = DropSlot;
					}
				}

				if (EmptySlot != -1)
				{
					if (InvComp->GrabItem(EmptySlot, Known.Item))
					{
						KnownItems.RemoveAt(i);
					}
				}
				else
				{
					// full inventory fallback to avoid getting stuck
					KnownItems.RemoveAt(i);
				}
			}
		}
	}
}
