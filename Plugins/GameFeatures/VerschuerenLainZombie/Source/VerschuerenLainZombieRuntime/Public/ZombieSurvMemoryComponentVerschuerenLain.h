#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemType.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.generated.h"

class ABaseItem;

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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VERSCHUERENLAINZOMBIERUNTIME_API UZombieSurvMemoryComponentVerschuerenLain : public UActorComponent
{
	GENERATED_BODY()

public:	
	UZombieSurvMemoryComponentVerschuerenLain();

	// add or update item
	void AddOrUpdateItem(ABaseItem* Item);

	// remove item
	void RemoveItem(ABaseItem* Item);

	// get known items
	const TArray<FMemoryItemVerschuerenLain>& GetKnownItems() const { return KnownItems; }

	// find closest item of type
	bool FindClosestItem(const FVector& Origin, EItemType Type, FVector& OutLocation, ABaseItem*& OutItem);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY()
	TArray<FMemoryItemVerschuerenLain> KnownItems;
};
