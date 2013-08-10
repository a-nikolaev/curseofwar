---
layout: page
title: How to play
---

# How the game works

Normally, the game starts with 4 small countries in the corners of the map.
You start as the ruler of the Green country, and your goal is to conquer
the whole map.

Tiles
-----

The map is made of hexagonal tiles. They are:

`/\^` Mountains, cannot be populated
<br />
`/$\` Gold mines, cannot be populated too.
This is the source of gold for your country.
To control a mine, surround it with your army
<br />
` n ` Villages, have the slowest population growth (+10%)
<br />
`i=i` Towns, average population growth (+20%)
<br />
`W#W` Fortresses, high population growth (+30%)
<br />
` - ` Grassland, normal habitable territory

People
------

People are your primary resource. Thriving popluation is essential for your
victory.

Every tile can support at most 499 people from each country.
When people from more than one country occupy the same tile, they fight.
The country that has the highest population at the tile is called the tile
owner.

The population of the tile owner is shown on all grassland tiles as follows:

` . ` 1 - 3 citizens <br />
`.. ` 4 - 6 <br />
`...` 7 - 12 <br />
` : ` 13 - 25 <br />
`.: ` 26 - 50 <br />
`.:.` 51 - 100 <br />
` ::` 101 - 200 <br />
`.::` 201 - 400 <br />
`:::` 400 - 499 

People migrate from highly populated tiles to less populated tiles.

Population grows only in cities. In villages by 10%, in towns by 20%, and
fortresses by 30% every simulation step. By controlling cities, you control
their surrounding territory.

Flags
-----

Every country can put flags on the map. Flags change migration rates,
so that people of your country start gathering at the tiles with flags.
Player's flags are shown as white "`P`" on the right side of a tile.
Flags of the computer opponents are shown as "`x`" on the left side of a tile.
The flags can be used to increase population of your own cities, as well as
for conquering foreign territories.

When countries are fighting for a city, and if the damage to the defender's
army is significant, the city can be destroyed: A fortress becomes a town,
a town becomes a village, and the village gets completely burnt by the invaders.

Countries
---------

Computer opponents differ in personality, and it affects the way they fight.

Controls
========

`Arrow keys` and `[H]`, `[J]`, `[K]`, `[L]` are for moving the cursor
<br />
`[R]` or `[V]`  to build village -> town -> castle
<br />
`[Space]`    to add/remove a flag
<br />
`[X]`        to remove all your flags
<br />
`[C]`        to remove a half of your flags
<br />
`[S]` slower
<br />
`[F]` faster
<br />
`[Q]` quit

Multiplayer
===========

To start a server for two players:
    
    $ ./curseofwar -E 2

To start a client and connect to the server:

    $ ./curseofwar -C <server's IP>

To specify ports, use `-e` option for server's port, and `-c` option for
client's port. By default, servers are using port `19140`, and clients are
using port `19150`.

Examples:
Start a server for a single client using port `11111`

    $ ./curseofwar -E 1 -e 11111

To connect to it:

    $ ./curseofwar -C <server's IP> -e 11111

Alternatively, to connect to it using port `12345` on the client's side:

    $ ./curseofwar -C <server's IP> -c 12345 -e 11111

Note that all needed map options must be setup when you start a server,
the map and other data are transmitted to clients, once they are connected.

Example:
Server for 3 clients, no computer opponents, hexagonal map, and equal
conditions for all:

    $ ./curseofwar -E3 -l3 -S hex -i0

Game speed cannot be changed by a client, so it must be set initially by
the server. Not all data is sent to clients (e.g. info about population is
not sent in full).

Multiplayer mode is at relatively early development stage. Changes may occure
at any moment. When you play with other people, make sure that you are using
the same version of the game. Hopefully, game's client-server communication
protocol will be improved in future. All communication is made via UDP.

Please, report you problems with multiplayer.
