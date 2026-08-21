# How Human Authors Make Heroes III Maps Interesting

## Corpus and method

- Scanned 224 maps in one batch: 65 HotA, 56 SoD, 48 RoE, 55 AB.
- Sizes: 36x36: 46, 72x72: 101, 108x108: 50, 144x144: 23, 180x180: 2, 216x216: 1, 252x252: 1.
- 160 maps have exact full-object decoding. The 64 HotA 1.8 maps have exact core headers, terrain, template tables, and declared object totals, with template-level content classification.
- No map contains the standard Random Map Generator description signature.

## Repeated design patterns

1. **Give the world a spatial identity.** 147 maps (66%) use an underground layer, while 114 (51%) devote at least 15% of their tiles to water. Authors use layers, coasts, terrain borders, roads, and rivers to divide a square grid into memorable regions.

2. **Replace simple elimination with a concrete job.** At least 51 maps use a decoded special victory condition. Flagging dwellings or mines, transporting an artifact, defeating one target, building the Grail, and resource races turn exploration into a directed campaign.

3. **Gate progress visibly.** 165 maps expose quest-related structures and 210 use travel-network objects. Border guards, tents, seer huts, garrisons, monoliths, subterranean gates, and ship routes let authors reveal the map in stages without making it a straight corridor.

4. **Alternate reliable income with optional danger.** Among fully decoded maps, the median counts are 259 economy objects, 72 monster objects, 24 artifacts, and 12 banks. This recurring rhythm creates expansion choices: take safe income now or spend movement and army strength on a high-value fight.

5. **Use asymmetry to create roles.** Human maps frequently mix fixed factions, unequal starting regions, isolated AI powers, and different access to water or underground. Fairness comes from compensating advantages and timing, not necessarily mirrored geometry.

6. **Make landmarks carry rules.** Towns, mines, creature banks, quest gates, monolith hubs, named heroes, and unusual terrain islands are both visual anchors and strategic decisions. A good region can be described by what the player is trying to reach there.

7. **Write story into the route.** 188 maps show event/sign/story structures in the available analysis. Messages work best when tied to crossing a border, visiting an object, taking a town, or meeting a named hero, so narrative arrives at the moment the geography changes.

8. **Control pacing through distance and guard strength.** Early zones cluster basic resources and low-risk pickups; connection points concentrate guards; remote branches hold artifacts, banks, prisons, or objectives. The map therefore teaches, tests, and rewards in spatial order.

## Practical recipe for generated scenarios

Start with a strategic graph before painting terrain: starting zones, expansion zones, contested hubs, optional reward branches, and the final objective. Give every connection a purpose and every dead end a reward. Then assign terrain identities, economy budgets, guard tiers, travel links, quests, and timed story beats. Finally test travel time and army growth for every playable start; visual symmetry is optional, but opportunity and tempo must be intentional.

## Limits

The reports infer design intent from map structure; they do not judge actual battle difficulty or balance without simulating armies and player routes. HotA 1.8 object bodies require a newer serializer than the available released batch converter, so those maps are labeled `template-presence` wherever instance-level category counts would otherwise overstate precision.
