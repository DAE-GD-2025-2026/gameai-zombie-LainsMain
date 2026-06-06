#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemType.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.generated.h"

class ABaseItem;
class ABaseZombie;
class AHouse;

USTRUCT(BlueprintType)
struct FHouseMemoryVerschuerenLain
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AHouse* House{nullptr};

	UPROPERTY(BlueprintReadOnly)
	bool bExplored{false};

	UPROPERTY(BlueprintReadOnly)
	FVector EntryPoint{FVector::ZeroVector};
};

USTRUCT(BlueprintType)
struct FMemoryItemVerschuerenLain
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ABaseItem* Item{nullptr};

	UPROPERTY(BlueprintReadOnly)
	FVector Location{FVector::ZeroVector};

	UPROPERTY(BlueprintReadOnly)
	EItemType ItemType{EItemType::Garbage};

	UPROPERTY(BlueprintReadOnly)
	int Value{0};
};

USTRUCT(BlueprintType)
struct FMemoryZombieVerschuerenLain
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ABaseZombie* Zombie{nullptr};

	UPROPERTY(BlueprintReadOnly)
	FVector LastKnownLocation{FVector::ZeroVector};

	UPROPERTY(BlueprintReadOnly)
	bool bSensed{false};

	UPROPERTY(BlueprintReadOnly)
	float LastTimeSeen{0.f};
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VERSCHUERENLAINZOMBIERUNTIME_API UZombieSurvMemoryComponentVerschuerenLain : public UActorComponent
{
	GENERATED_BODY()

public:	
	UZombieSurvMemoryComponentVerschuerenLain();

	virtual void BeginPlay() override;

	// add or update item
	void AddOrUpdateItem(ABaseItem* Item);

	// remove item
	void RemoveItem(ABaseItem* Item);

	// get known items
	const TArray<FMemoryItemVerschuerenLain>& GetKnownItems() const { return KnownItems; }

	// find closest item of type
	bool FindClosestItem(const FVector& Origin, EItemType Type, FVector& OutLocation, ABaseItem*& OutItem);

	// add zombie
	void AddZombie(ABaseZombie* Zombie);

	// remove zombie
	void RemoveZombie(ABaseZombie* Zombie);

	// find closest zombie
	bool FindClosestZombie(const FVector& Origin, ABaseZombie*& OutZombie);

	// get next unexplored house (closest one)
	bool GetNextUnexploredHouse(const FVector& Origin, FVector& OutLocation, AHouse*& OutHouse);

	// mark house as explored
	void MarkHouseExplored(AHouse* House);

	// mark closest house as explored (optimized)
	void MarkClosestHouseExplored(const FVector& Origin, float Radius);

	// check if location is inside any active purge zone
	bool IsLocationInPurgeZone(const FVector& Location, float SafetyMargin = 200.f);

	// check if a path intersects any active purge zones in 2D
	bool DoesPathIntersectPurgeZone(const TArray<FVector>& Path, float SafetyMargin = 150.f);

	// helper to safely check if a zombie is dead using its health component
	static bool IsZombieDead(ABaseZombie* Zombie);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY()
	TArray<FMemoryItemVerschuerenLain> KnownItems;

	UPROPERTY()
	TArray<FMemoryZombieVerschuerenLain> KnownZombies;

	UPROPERTY()
	TArray<FHouseMemoryVerschuerenLain> HousesMemory;

	UPROPERTY(EditAnywhere, Category = "Memory")
	FName ExploringHouseKeyName{TEXT("ExploringHouse")};

	UPROPERTY(EditAnywhere, Category = "Memory")
	float PeriodicScanInterval{15.f};

	UPROPERTY(EditAnywhere, Category = "Memory")
	FName InPurgeZoneKeyName{TEXT("InPurgeZone")};

	UPROPERTY(EditAnywhere, Category = "Memory")
	FName Needs360ScanKeyName{TEXT("Needs360Scan")};

	int LastHealth{-1};
	float ScanTimer{0.f};
};
