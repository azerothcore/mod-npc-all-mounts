# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore
## NPC - All Mounts #
- Latest build status with azerothcore: [![Build Status](https://github.com/azerothcore/mod-npc-all-mounts/workflows/core-build/badge.svg?branch=master&event=push)](https://github.com/azerothcore/mod-npc-all-mounts)

_This module was created for [StygianCore](https://rebrand.ly/stygiancoreproject). A World of Warcraft 3.3.5a Solo/LAN repack by StygianTheBest | [GitHub](https://rebrand.ly/stygiangithub) | [Website](https://rebrand.ly/stygianthebest))_
_This module has now being ported to AzerothCore by gtao725._

### Description ###

- Adds an NPC that will teach all available mounts to the player
- 
## Notice:

Due to the uniquiness of the module you will get this message on the worldconsole, but nothing is broken.
It is due to the npc not exactly having a gossip menu in the database, the script handles the gossip menu because of the additional locales.

![image](https://user-images.githubusercontent.com/16887899/154320677-b55f8528-6129-46e2-902f-39d3465244b7.png)

Please add the adjustment to the conf to have that error message not show up:
```
#    Creatures.CustomIDs
#        Description: The list of custom creatures with gossip dialogues hardcoded in core,
#                     divided by "," without spaces.
#                     It is implied that you do not use for these NPC dialogs data from "gossip_menu" table.
#                     Server will skip these IDs during the definitions validation process.
#        Example:     Creatures.CustomIDs = "190010,55005,999991,25462,98888,601014" - Npcs for Transmog, Guild-zone, 1v1-arena, Skip Dk,
#                                                                                      Racial Trait Swap, NPC - All Mounts Modules
#        Default:     ""

Creatures.CustomIDs = "190010,55005,999991,25462,98888,601014"
```

### To-Do ###

- Remove all mounts not compatible with 3.3.5a

### Data ###

- Type: NPC
- Script: All_Mounts_NPC
- Config: Yes
- SQL: Yes
    - NPC ID: 601014


### Version ###

- v2019.04.15 - Ported to AzerothCore by gtao725 (https://github.com/gtao725/)
- v2019.02.14 - Update AI, Fix Bengal Tiger, Remove Invalid Mounts
- v2018.12.15 - Updated for StygianCore
- v2017.09.03 - Added Bengal Tiger + Tiger Riding
- v2017.08.01 - Release


### CREDITS

![Styx](https://stygianthebest.github.io/assets/img/avatar/avatar-128.jpg "Styx")
![StygianCore](https://stygianthebest.github.io/assets/img/projects/stygiancore/StygianCore.png "StygianCore")

##### This module was created for [StygianCore](https://rebrand.ly/stygiancoreproject). A World of Warcraft 3.3.5a Solo/LAN repack by StygianTheBest | [GitHub](https://rebrand.ly/stygiangithub) | [Website](https://rebrand.ly/stygianthebest))

#### Additional Credits

- [Blizzard Entertainment](http://blizzard.com)
- [TrinityCore](https://github.com/TrinityCore/TrinityCore/blob/3.3.5/THANKS)
- [SunwellCore](http://www.azerothcore.org/pages/sunwell.pl/)
- [AzerothCore](https://github.com/AzerothCore/azerothcore-wotlk/graphs/contributors)
- [OregonCore](https://wiki.oregon-core.net/)
- [Wowhead.com](http://wowhead.com)
- [OwnedCore](http://ownedcore.com/)
- [ModCraft.io](http://modcraft.io/)
- [MMO Society](https://www.mmo-society.com/)
- [AoWoW](https://wotlk.evowow.com/)
- [More credits are cited in the sources](https://github.com/StygianTheBest)

### LICENSE

This code and content is released under the [GNU AGPL v3](https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3).
