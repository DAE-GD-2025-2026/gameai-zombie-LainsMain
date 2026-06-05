#include "StudentPerceptorVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Zombies/BaseZombie.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"

UStudentPerceptorVerschuerenLain::UStudentPerceptorVerschuerenLain()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptorVerschuerenLain::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptorVerschuerenLain::OnPerceptionUpdated);
	}
}

void UStudentPerceptorVerschuerenLain::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// check what sense triggered this
	auto SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(World, Stimulus);
	bool bIsSight = (SenseClass == UAISense_Sight::StaticClass());
	bool bIsDamage = (SenseClass == UAISense_Damage::StaticClass());

	bool bSensed = Stimulus.WasSuccessfullySensed();

	if (bIsSight)
	{
		// check if item
		if (ABaseItem* Item = Cast<ABaseItem>(Actor))
		{
			FString TypeStr = TEXT("Garbage");
			switch (Item->GetItemType())
			{
			case EItemType::Food: TypeStr = TEXT("Food"); break;
			case EItemType::Medkit: TypeStr = TEXT("Medkit"); break;
			case EItemType::Shotgun: TypeStr = TEXT("Shotgun"); break;
			case EItemType::Pistol: TypeStr = TEXT("Pistol"); break;
			default: break;
			}

			FString Msg = FString::Printf(TEXT("sight: %s item %s (val: %d) at %s"),
				bSensed ? TEXT("saw") : TEXT("lost"),
				*TypeStr, Item->GetValue(), *Item->GetActorLocation().ToString());

			GEngine->AddOnScreenDebugMessage(-1, 5.f, bSensed ? FColor::Green : FColor::Orange, Msg);

			if (bSensed)
			{
				if (auto MemoryComp = GetOwner()->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>())
				{
					MemoryComp->AddOrUpdateItem(Item);
				}
			}
		}
		// check if zombie
		else if (ABaseZombie* Zombie = Cast<ABaseZombie>(Actor))
		{
			FString Msg = FString::Printf(TEXT("sight: %s zombie at %s"),
				bSensed ? TEXT("saw") : TEXT("lost"),
				*Zombie->GetActorLocation().ToString());

			GEngine->AddOnScreenDebugMessage(-1, 5.f, bSensed ? FColor::Red : FColor::Orange, Msg);

			if (auto MemoryComp = GetOwner()->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>())
			{
				if (bSensed)
				{
					MemoryComp->AddZombie(Zombie);
				}
				else
				{
					MemoryComp->RemoveZombie(Zombie);
				}
			}
		}
	}
	else if (bIsDamage)
	{
		// hit by zombie
		FString Msg = FString::Printf(TEXT("damage: took %f damage from %s"),
			Stimulus.Strength, *Actor->GetName());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Msg);
	}
}
