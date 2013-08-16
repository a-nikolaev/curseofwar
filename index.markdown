---
layout: default
title: Home
---

[![screenshot3](http://i.imgur.com/QhsgqRbs.png)](http://i.imgur.com/QhsgqRb.png)
[![screenshot2](http://i.imgur.com/JoVcPvTs.png)](http://i.imgur.com/JoVcPvT.png) 
[![gif-animation](http://i.imgur.com/K2AV6zSs.gif)](http://i.imgur.com/K2AV6zS.gif) 
[![screenshot-sdl1](http://i.imgur.com/i4CgNVes.png)](http://i.imgur.com/i4CgNVe.png) 
[![screenshot-sdl2](http://i.imgur.com/eUFmyjUs.png)](http://i.imgur.com/eUFmyjU.png) 
[![screenshot-sdl3](http://i.imgur.com/YM0nYQvs.png)](http://i.imgur.com/YM0nYQv.png) 

**Curse of War** is a fast-paced action strategy game for Linux originally 
implemented using ncurses user interface. An SDL graphical version is also available.

Unlike most RTS, you are not controlling units, but focus on high-level 
strategic planning: Building infrastructure, securing resources, 
and moving your armies.

**Influences:** Globulation2, Liquidwar, Settlers, StarCraft, various tty games, roguelikes.

<!--
## Videos

[![](http://i.imgur.com/mN7lBYi.png)](http://youtu.be/FZH4qbIxpx4) 
[![](http://i.imgur.com/4RSZixh.png)](http://youtu.be/EzGuR66gemw) 
[![](http://i.imgur.com/yxQPCUL.png)](http://youtu.be/Egh3Jw-XafU)
-->

# Install

[Installation instructions]({{ site.baseurl }}/install)

# Quick Start

A good and easy mode to start playing:

    $ curseofwar -i4 -q1 -dee

Or, on a smaller map:

    $ curseofwar -i4 -q1 -dee -W16 -H16

##Rules 

1. Cities (`W#W`, `i=i`, and ` n `) increase your country's population
2. Gold mines (`/$\`) give you gold, which is used to build new cities.
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

## Our IRC
Join #curseofwar on irc.freenode.net 

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
