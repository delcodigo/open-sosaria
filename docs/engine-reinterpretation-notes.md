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

## Town Transactions, Drinking, and Pond Use

### Starting a Town Transaction

- Town shopping starts with the **T** command.
- The player must be within the 3x3 area around a valid merchant/shop tile in the town collision map.
- Transactions are blocked while town hostility/combat is active (`enemyEncounter.monsterId > 0`).
- After starting a transaction, the player must choose **B**uy or **S**ell.
- Any other non-`T` key at that prompt cancels the transaction.
- The shop type is determined by the player's position inside town, not by directly targeting a specific NPC.

### Town Shop Regions

- **Transport shop**: `x = 4..9`, `y = 4..7`
- **Magic shop**: `x = 22..25`, `y = 1..6`
- **Pub**: `x = 30..35`, `y = 4..7`
- **Armory**: `x = 4..9`, `y = 14..17`
- **Weapon shop**: `x = 11..16`, `y = 14..17`
- **Food shop**: `x = 22..26`, `y = 12..17`

### Transport Shop

- The transport shop supports **buying only**. Selling is always refused.
- Transport cost is:

$$
\left\lfloor \frac{200 - intelligence}{80} \times (vehicleId \times 4)^2 \right\rfloor
$$

- Purchased vehicles are spawned onto an adjacent overworld tile near the town exit position, and the corresponding vehicle inventory count is incremented.
- The shop checks only the **orthogonal** adjacent overworld tiles.
- **Horse** and **cart** use an empty adjacent **land** tile.
- **Raft** and **frigate** are offered only when an empty adjacent **water** tile exists.
- **Air car** and **shuttle** are offered only when an empty adjacent **land** tile exists and `player.time > 3000`.
- If the required placement tile is unavailable, the purchase is rejected even if the option was shown.
- The current implementation accepts only menu digits **1..6**.
- There is an implementation quirk in the late-game transport menu: the displayed prices for the last two transports are generated from higher cost ids than the ids actually charged on purchase.

### Magic Shop

- The magic shop supports **buying only**. Selling is always refused.
- **PRAY** is not sold here; the purchasable set is spell ids **1..10**.
- Spell cost is:

$$
\left\lfloor \frac{200 - wisdom}{200} \times spellId \times 5 \right\rfloor
$$

- Buying a spell requires both:
	- enough **gold** to pay the price
	- enough **experience points** (`eptns`) to pay the same price again
- On purchase, the price is subtracted from both **gold** and **eptns**.
- Spell selection uses the spell's leading letter.
- Ladder spells require a two-step input:
	- first **L**
	- then **U** for Ladder Up or **D** for Ladder Down
- Spells with ids **7..10** are class-restricted and can only be bought when `player.type == 3`.

### Armory

- The armory supports both **buying** and **selling**.
- Armor cost is:

$$
\left\lfloor \frac{200 - intelligence}{200} \times 50 \times armorId \right\rfloor
$$

- Armor selection uses the first letter of the armor name.
- The first three armors are available immediately.
- The last two suits become available only when `player.time > 2000`.
- Buying armor is blocked if `player_getEncumbrance() > 0`.
- Sell price is:

$$
\left\lfloor \frac{charisma}{50} \times buyCost \right\rfloor
$$

- During selling, the shop quotes a price before checking inventory ownership.
- Actual ownership is checked only when the player confirms the sale.
- If the sold armor was currently equipped and the last copy is removed, equipped armor is reset to the default slot.

### Weapon Shop

- The weapon shop supports both **buying** and **selling**.
- Weapon cost is:

$$
\left\lfloor \frac{200 - intelligence}{200} \times weaponId^2 + 5 \right\rfloor
$$

- Weapon selection uses the first **two letters** of the weapon name.
- Buying weapons is blocked if `player_getEncumbrance() > 0`.
- Shop inventory is gated by a derived weapon-era value:

$$
time = \left\lfloor \frac{player.time}{3000} \times 2 + 1 \right\rfloor
$$

- That value is capped at **5**, then increased by **1** when the current overworld tile type is even, producing a final availability band from **1** to **6**.
- Different weapon groups are shown depending on that band, so the stock depends on both world progression and the overworld terrain under the town entrance position.
- As with armor, sell price is based on charisma and inventory ownership is only validated after the player confirms the sale.
- If the sold weapon was currently equipped and the last copy is removed, equipped weapon falls back to the default slot.

### Food Shop

- The food shop supports **buying only**. Selling is always refused.
- Food price is a flat per-transaction value:

$$
\left\lfloor 5 - \frac{intelligence}{20} \right\rfloor
$$

- The player then enters a single digit **1..9**.
- The selected digit buys **10 to 90 food**.
- Entering **0** cancels the purchase.
- In the current implementation, the player pays the flat food cost only **once**, regardless of how many tens of food were selected.

### Drinking at the Pub

- Buying at the pub means buying a **drink**.
- Each drink costs **1 gold**.
- After payment, `drunkLevel` is incremented by **1**.
- `drunkLevel` persists for the current town visit and resets when the town scene is initialized again.
- If `drunkLevel` exceeds either `stamina / 5` or `wisdom / 5`, and the wench is within a radius of **1.5** tiles, the player is robbed:
	- gold is set to **0**
	- wisdom is reduced by **1**
	- wisdom is clamped to a minimum of **5**
- Otherwise, the pub either ends quietly or produces a random gossip sequence.
- The gossip table is selected uniformly from **10** outcomes, with some entries spanning multiple console lines.

### Using the Pond

- The pond is not a dedicated command; it is used through the town **Drop** command by dropping **gold**.
- Gold entry accepts up to **6 digits** and is confirmed with **Enter**.
- If the entered amount exceeds the player's current gold, the action fails.
- The pond effect is active only in the southeast town region where `player.px >= 29` and `player.py >= 13`.
- When gold is dropped there:
	- player health increases by:

$$
\left\lfloor goldDropped \times 1.5 \right\rfloor
$$

- the dropped gold is removed from the player
- On the **first successful pond donation of that town visit**, the player also receives **4 daggers**.
- That first-time reward is tracked with `droppedGold` and resets when the town scene is re-entered.
- Dropping gold outside the pond area still removes the gold, but provides no healing or dagger reward.

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
