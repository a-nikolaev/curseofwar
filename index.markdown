---
layout: default
title: Home
---

**Curse of War** is a fast-paced action strategy game for Linux implemented using 
ncurses user interface.

Unlike most RTS, you are not controlling units, but focus on high-level 
strategic planning: Building infrastructure, securing resources, 
and moving your armies.

# Install

[Installation instructions]({{ site.baseurl }}/install)

# Quick Start

A good and easy mode to start playing:

    $ curseofwar -i4 -q1 -dee

Or, on a smaller map:

    $ curseofwar -i4 -q1 -dee -W16 -H16

##Rules 

1. Cities (`W#W`, `i=i`, and ` n `) increase your country's population
2. Gold mines (`/$\`) give your gold, which is used to build new cities.
3. Put flags on the map to move your army. Press space to add and remove flags.
4. You play as a ruler of the green country.
5. Computer opponents have different personalities and different approach to control their armies
6. Game conditions can be adjusted with command line arguments, see `$ curseofwar -h`

##Controls

`Arrow keys` and `[H]`, `[J]`, `[K]`, `[L]` are for moving the cursor
<br />
`[R]` or `[V]`  to build village -> town -> castle
<br />
`[Space]`    to add/remove a flag
<br />
`[X]`        to remove all your flags
<br />
`[C]`        to remove a half of your flags

## Multiplayer
To start a server for two players:

    $ curseofwar -E 2

To start a client and connect to the server:

    $ curseofwar -C <server's IP> 
                                       
To play with another person, you start a server, and both players start their clients, connecting to this server (i.e. if you run the server, you should connect to your own IP, `127.0.0.1`). See the last section of the [README](https://github.com/a-nikolaev/curseofwar#readme) file for details.

<!--
<div id="home">
  <h1>The main header</h1>
  <ul class="posts">
    {% for post in site.posts %}
      <li><span>{{ post.date | date_to_string }}</span> &raquo; <a href="{{ post.url }}">{{ post.title }}</a></li>
    {% endfor %}
  </ul>
</div>
-->
