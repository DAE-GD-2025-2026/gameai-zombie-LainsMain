# Game AI Zombie Survival Agent

This plugin-based AI was developed for the **Algorithms 2** course at **Digital Arts and Entertainment (DAE)**. It implements a fully autonomous survivor agent designed to survive as long as possible in a simulated zombie apocalypse.

All logic and assets are encapsulated in the Game Feature Plugin:  
📂 `Plugins/GameFeatures/VerschuerenLainZombie/`

---

## 🏗️ How the Survivor Works

The agent uses a **Perception-Memory-Behavior Tree** loop to make decisions dynamically without querying global game states.

```mermaid
graph LR
    Perception[AIPerception] --> Memory[Memory Component]
    Memory --> Blackboard[Blackboard Keys]
    Blackboard --> BehaviorTree[Behavior Tree]
```

### 1. Perception & Memory
* **Event-Driven Sensing:** Captures sight (zombies/items) and damage (hit direction) events and sends them to memory.
* **Persistent Knowledge:** Remembers known item locations, trackable threats (removes dead/lost zombies), and unexplored houses.
* **Purge Zone Avoidance:** Detects if the survivor's current location or planned navigation path intersects a Purge Zone, triggering an immediate sprint to safety.
* **Auto-Inventory:** Automatically grabs nearby items, dropping lower-priority duplicates or empty weapons to optimize slot usage.

### 2. Decision Tree Priorities
The Behavior Tree executes actions in three prioritized phases:

1. **Emergency Response (Highest):**
   * Uses a medkit if health is $< 30\%$.
   * **Combat:** Faces threats, checks line of sight, and shoots (using Shotgun at close range, Pistol at mid-range).
   * **Fleeing:** Runs toward known guns, houses, or searches 8 directions for a physical hiding spot that blocks visibility.
2. **Need Acquisition:**
   * Collects items based on current needs: **Health (Medkit) > Stamina (Food) > Defense (Weapons) > General Items**.
3. **Exploration (Lowest):**
   * Dynamically routes to the closest unexplored house. 
   * Conserves stamina by walking during exploration (only sprints when fleeing or escaping purge zones).
   * Performs 360-degree scans when taking damage or periodically to gather information.

---

## 🏷️ Suffix Requirement
To compile in the shared test environment, all custom files, C++ classes, and blueprint assets are uniquely suffixed with `VerschuerenLain`.
