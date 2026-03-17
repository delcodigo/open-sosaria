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
