# Engine Reinterpretation Notes

This document captures observed behavior and implementation decisions gathered while reinterpreting the original engine behavior for `open-sosaria`.

It is intended to be updated incrementally as more findings are confirmed.

## Timing and Waiting

- Original Apple II keyboard wait behavior appears to use **200 keyboard wait iterations**.
- In this engine, that behavior has been reinterpreted as a **5 second wait**, which produces similar perceived behavior when compared against emulator execution.
- Waiting in place consumes **0.05 pieces of food**.
- Waiting in place advances time by **0.5 units**.
- Manual waiting in place does **not** apply any penalty

## Movement Cost Model

Movement consumes food and advances time based on `transportType`.

### Food Consumption

Food consumed per move:

$$
\frac{7 - transportType}{14}
$$

### Time Progression

Time advanced per move:

$$
\frac{7 - transportType}{7}
$$

## Character Generator Constraints

- Each stat must be allocated within the range of **10 to 20 points**.
- Player names may be up to **15 characters** long.
- Food, HP, and health start at **100**.
- Experience starts at **1**.
- Initial player position is **(40, 40)**.

## Inventory Items

- There are **5** armors:
	- LEATHER ARMOR
	- CHAIN MAIL
	- PLATE MAIL
	- VACUUM SUIT
	- REFLECT SUIT
- There are **6** vehicles:
	- HORSE
	- CART
	- RAFT
	- FRIGATE
	- AIR CAR
	- SHUTTLE
- There are **15** weapons:
	- DAGGER
	- MACE
	- AXE
	- ROPE & SPIKES
	- SWORD
	- GREAT SWORD
	- BOW & ARROWS
	- AMULET
	- WAND
	- STAFF
	- TRIANGLE
	- PISTOL
	- LIGHT SWORD
	- PHAZOR
	- BLASTER
- There are **11** spells:
	- PRAY
	- OPEN
	- UNLOCK
	- PROJECTILE
	- STEAL
	- LADDER DOWN
	- LADDER UP
	- BLINK
	- CREATE
	- DESTROY
	- KILL
- There are **4** gems:
	- RED GEM
	- GREEN GEM
	- BLUE GEM
	- WHITE GEM

## Ready Command Input Behavior (Original Ultima)

- Pressing **R** prompts for category selection: **(W)eapons**, **(A)rmors**, or **(S)pells**.
- Each category uses different matching rules:
	- **Weapons** require entering the first **two letters** of the weapon name.
	- **Armors** require only **one letter**.
	- **Spells** usually require only **one letter**, except ladder spells:
		- Enter **L** first.
		- Then enter **U** (Ladder Up) or **D** (Ladder Down).
- Each category also has a default hidden selection:
	- **Weapons** default: **Hands**.
	- **Armor** default: **Skin**.
	- **Spells** default: **Pray**.
- These defaults are selectable through readying behavior, but they are not shown as normal inventory items in the Ztats screen.

## Overworld Spell Restrictions

- Only **3** spells are usable on the overworld: **pray**, **projectile**, and **kill**.

### Original `cast_spell` Behavior Findings

- Casting starts by prompting with `CAST <spell name>`.
- Spell inventory validation is skipped for **PRAY** (index 0), but applied to other spells.
- A spell charge is consumed immediately after validation, before spell-specific resolution.
- Spells other than **PRAY**, **PROJECTILE**, and **KILL** fail with a "dungeon spell only" message.

#### PRAY

- PRAY performs one of **3** random outcomes.
- Outcome 1 priority:
	- Remove monster if one is present.
	- Otherwise, if gold is greater than 20, reduce gold by 20.
	- If health is below 10, set health to 10 and print `SHAZAM!!!!`.
	- Else if food is below 10, set food to 10 and print `SHAZAM!!!!`.
	- Otherwise print `HMMM?...NO EFFECT??`.
- Outcome 2 priority:
	- If health is below 10, set health to 10 and print `SHAZAM!!!!`.
	- Else if food is below 10, set food to 10 and print `SHAZAM!!!!`.
	- Otherwise print `HMMM?...NO EFFECT??`.
- Outcome 3:
	- If food is below 10, set food to 10 and print `SHAZAM!!!!`.
	- Otherwise print `HMMM?...NO EFFECT??`.

#### PROJECTILE

- Requires a valid target; otherwise prints `NO TARGET!`.
- Damage is computed as half of intelligence, with an additional bonus when the equipped weapon index is between 8 and 11.
- The result is applied as direct monster HP reduction.

#### KILL

- Requires a valid target; otherwise prints `NO TARGET!`.
- Sets monster health to a killed state immediately.

## Bottom Console Stat Overflow

- If any stat shown in the bottom console exceeds **99999**, it is rendered as **`*****`**.
- The original numeric value is shown again automatically once it falls back below **100000**.

## Overworld

- The overworld map is divided into **4 chunks**: `bterra0`, `bterra1`, `bterra2`, `bterra3`.
- Although each realm's playable area is a **64x64** chunk, the underlying data for each chunk is actually **86x86** tiles. The additional outer area is filled with water tiles, effectively increasing the separation and travel distance between the different realms.

## Overworld Combat Flow

- After each player action, overworld combat resolution runs on the next update cycle.
- Combat state is tracked with `enemyEncounter.monsterId`:
	- Active encounter: monster id between **6** and **20**.
	- No active encounter: ids outside that range.
- Monster encounters with ids **6..20** are the overworld encounter set.
- If there is no active encounter, the engine attempts to spawn a new enemy group.
- If there is an active encounter, the enemy group performs an attack pass against the player.

### Encounter Spawn and Group Size

- Spawn checks are tile-dependent and probabilistic:
	- Base early return when `rand01() > 0.1`.
	- Extra suppression on tile type 1 when `rand01() > 0.5`.
- Spawned monster id range depends on tile category.
- Group count is:

$$
\left\lfloor rand01()^2 \times groupSize\right\rfloor + 1
$$

where `groupSize` comes from `enemyDefinitions[monsterId].group`.

### Enemy Attack Resolution

- During enemy retaliation, each enemy in the group rolls a hit independently.
- Player defense uses armor, with extra defense when riding vehicles with ids **4..6**: **FRIGATE**, **AIR CAR**, and **SHUTTLE**.
- Each successful enemy hit reduces player health by:

$$
\left\lfloor rank \times rand01() + 1\right\rfloor
$$

where `rank` is the attacking monster rank.

### Movement While in Combat

- Moving while an encounter is active first runs an escape/dodge check.
- On successful dodge/escape, the encounter is cleared and enemy rendering is disabled.
- On failed dodge/escape:
	- Movement is blocked for that action.
	- Enemy rendering is enabled.
	- An encounter message is printed.

### Fleeing and Weapon Reach Constraints

- Fleeing is a valid and sometimes necessary option because weapon viability is monster-dependent.
- In melee attack flow:
	- Monsters with id **< 10** are treated as water monsters.
	- Monster id **12** is treated as hidden archers.
	- Weapons with indices **< 7**, **11**, and **13** are treated as melee-only for this reach check.
- If a melee-only weapon is used against those monster sets, the attack yields an immediate no-damage result.

### Group HP Model (First Enemy vs Following Enemies)

- Initial encounter HP (effectively the first enemy in the group) is generated in spawn logic using formulas that include progression scaled by `player.time / 100`.
- After one enemy dies and the group still has members left, HP for the next enemy is regenerated with a different formula that scales progression by `player.time / 1000` and uses a lower baseline.
- Practical effect: the first enemy in a spawned group tends to be tougher, while subsequent enemies in the same group are generally weaker.

### Overworld Extra Commands

- `drop`, `get`, and `open` are not actually usable actions on the overworld.
- Even so, the game still accepts those commands and prints a specific feedback message for each one.

## Signpost Upgrade Spam Prevention
- To prevent players from repeatedly receiving upgrades from the same signpost, the engine tracks the last signpost interacted with and temporarily blocks duplicate rewards until a different signpost is checked.


## Resurrection Behavior

- Resurrection is triggered when the player's food or health drops to 0 or below.
- Upon resurrection:
	- All weapons are removed from the player's inventory.
	- Health is set to 99.
	- The player is dismounted from any vehicle.
	- Gold is set to 0.
	- Food is set to 20.
	- The player is relocated to a random position on the map.
		- In the original game, this could result in the player being stranded on water or a mountain, potentially leading to repeated starvation and death.
		- In this reimplementation, the player is moved to a safer area to avoid such softlocks.

## Save/Load Map Snapshot Behavior

- On save/load, the original behavior persists the current **64x64 map tile buffer**.
- This is important because vehicles (for example, horses) are encoded directly in map tiles instead of separate entities.
- Tile encoding uses nibbles:
	- **Lower nibble**: tile backup.
	- **Upper nibble**: vehicle value.
- While this is an interesting compact approach, in this engine reinterpretation vehicles are stored in a more traditional **entity list**.

## BEVERY Data Loading

- `BEVERY` is a saved snapshot of the Ultima I player-state memory region.
- In the original game, save/load works by `BSAVE`/`BLOAD` of memory starting at `$7800` (`30720`) for about 7.5 KB, with a few zero-page state bytes copied into that region before saving.
- That image contains both Applesoft-managed runtime data stored in the saved region and other state the game keeps in memory for the player/world session.
- This engine reinterpretation reads the `BEVERY` memory image and locates Applesoft variables and arrays inside it, then converts them into local engine data structures.
- The extraction path is implemented in `bever.c` and `bever.h`.

## Notes

These findings are based on reinterpretation and observed behavior, and should be refined further as additional engine details are validated.
