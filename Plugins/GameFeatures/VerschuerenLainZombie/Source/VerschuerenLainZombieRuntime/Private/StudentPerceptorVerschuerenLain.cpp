#include "StudentPerceptorVerschuerenLain.h"
#include "Items/BaseItem.h"
#include "Zombies/BaseZombie.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "ZombieSurvMemoryComponentVerschuerenLain.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UStudentPerceptorVerschuerenLain::UStudentPerceptorVerschuerenLain()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	auto SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(World, Stimulus);
	bool bIsSight = (SenseClass == UAISense_Sight::StaticClass());
	bool bIsDamage = (SenseClass == UAISense_Damage::StaticClass());

	bool bSensed = Stimulus.WasSuccessfullySensed();

	if (bIsSight)
	{
		if (ABaseItem* Item = Cast<ABaseItem>(Actor))
		{
			if (bSensed)
			{
				if (auto MemoryComp = GetOwner()->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>())
				{
					MemoryComp->AddOrUpdateItem(Item);
				}
			}
		}
		else if (ABaseZombie* Zombie = Cast<ABaseZombie>(Actor))
		{
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
		if (auto MemoryComp = GetOwner()->GetComponentByClass<UZombieSurvMemoryComponentVerschuerenLain>())
		{
			ABaseZombie* ClosestZombie = nullptr;
			bool bHasZombie = MemoryComp->FindClosestZombie(GetOwner()->GetActorLocation(), ClosestZombie);
			if (!bHasZombie)
			{
				APawn* Pawn = Cast<APawn>(GetOwner());
				if (Pawn)
				{
					if (auto Controller = Pawn->GetController<AAIController>())
					{
						if (auto BlackboardComp = Controller->GetBlackboardComponent())
						{
							BlackboardComp->SetValueAsBool(Needs360ScanKeyName, true);
						}
					}
				}
			}
		}
	}
}
