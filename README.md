# YRMapsUpdater

`YRMapsUpdater` is a command-line tool which facilitates map updates to the CnCNet Client for Yuri's Revenge. It's primary function is to read data from all maps and MPMaps.ini in order to generate a new MPMaps.ini configuration file. It also provides the option to create a list of all maps missing from versionconfig.ini.

### Usage

Double-click the executable to run as normal. The first time you run it, you will be prompted to input the path to CnCNet and, if you choose to create a list of new maps and previews, versionconfig.ini. Once you input these paths they will be saved, and unless you move or delete PathsYRMU.ini, you will not be prompted to input them again.

If MPMapsBase.ini is not in the same directory as the executable, you will be prompted to input its correct path. This will not be saved, so it is recommended that you keep MPMapsBase.ini in the same directory as the executable.

THE APPLICATION DOES NOT RECOGNIZE UNICODE CHARACTERS. Directories which have accented characters in their names will not be recognized as valid. Before you run the application, ensure that its directory and your CnCNet directory are free of accented characters.

### MPMaps.ini Generation

`YRMapsUpdater` requires maps to have a valid name in order to process them. It searches both the map itself (using the "Name" tag under the section [Basic]) and the map's section in the original MPMaps.ini (using the "Description" tag) for a valid name. Name validity is determined by the regular expression `\[\d\] \S.+`.

The MPMapsBase.ini configuration file is also required in order to generate MPMaps.ini. This file specifies any information not present in the maps, such as forced options for gamemodes.

`YRMapsUpdater` searches maps for two sections which are parsed by the client, but not the game, namely [ForcedOptions] and [ForcedSpawnIniOptions] (see these entries in the original MPMaps.ini for examples). It also searches for tags under [Basic] which are parsed by the client:

```
[Basic]
IsCoopMission=true            ;If this is set, the following tags required
DisallowedPlayerSides=X,X...  ;Comma delimited list of factions (defined by integers) players cannot choose
EnemyHouse0=S,C,W             ;Side, color, waypoint of an AI opponent
...                           ;EnemyHouse0-7 are allowed, at least EnemyHouse0 is required
DisallowedPlayerColors=X,X... ;Comma delimited list of colors (defined by integers) players cannot choose
```

All of these sections and tags determine which options are available or predetermined in the client. Including them is optional, `YRMapsUpdater` will ignore them if they are not present.
