---
layout: page
title: Options
---

The game is configured via command line arguments.

`-W width`
Map width (default is 21)

`-H height`
Map height (default is 21)

`-S [rhombus|rect|hex]`
Map shape (rectangle is default). Max number of countries N=4 for
rhombus and rectangle, and N=6 for the hexagon.

`-l [2|3| ... N]`
Sets L, the number of countries (default is N).

`-i [0|1|2|3|4]`
Inequality between the countries (0 is the lowest, 4 in the highest).

`-q [1|2| ... L]`
Choose player's location by its quality: 1 = the best available
on the map, and L = the worst. Only in the singleplayer mode.

`-r`
Absolutely random initial conditions, overrides options `-l`, `-i`, and `-q`.

`-d [ee|e|n|h|hh]`
Difficulty level (AI) from the easiest to the hardest (default is normal).

`-s [p|sss|ss|s|n|f|ff|fff]`
Game speed from the slowest to the fastest (default is normal).

`-R seed`
Specify a random seed (unsigned integer) for map generation.

`-T`
Show the timeline.

`-E [1|2| ... L]`
Start a server for not more than L clients.

`-e port`
Server's port (19140 is default).

`-C IP`
Start a client and connect to the provided server's IP-address.

`-c port`
Clients's port (19150 is default).

`-v`
Display the version number

`-h`
Display help information


