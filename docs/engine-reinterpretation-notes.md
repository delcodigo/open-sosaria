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

## Overworld

- The overworld map is divided into **4 chunks**: `bterra0`, `bterra1`, `bterra2`, `bterra3`.

## Notes

These findings are based on reinterpretation and observed behavior, and should be refined further as additional engine details are validated.
