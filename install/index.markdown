---
layout: page
title: How to install the game
---

# Packages for popular Linux distributions

* Arch Linux: [community (ncurses)](https://www.archlinux.org/packages/community/x86_64/curseofwar/), 
[AUR (ncurses) git](https://aur.archlinux.org/packages/curseofwar-git/),
or [AUR (ncurses and SDL) git](https://aur.archlinux.org/packages/curseofwar-with-sdl-git/).
* CRUX: [ports](http://crux.nu/portdb/?a=repo&q=contrib) in repository contrib - ncurses and/or SDL.
* Debian: [unstable](http://packages.debian.org/unstable/main/curseofwar)
* Ubuntu: [PPA team](https://launchpad.net/~curseofwar) - ncurses and/or SDL.

# Build from sources
It's not hard to build the game from sources. 
  
Ncurses version:
    
    $ make

SDL version:

    $ make SDL=yes
  
If it works, see detailed instructions [how to build & install the game]({{ site.baseurl }}/build/).

## With CMake
If you like, you can build the game using CMake, see details in COMPILE-WITH-CMAKE.

# Windows
Download compiled game zip link.

Alternatively, you can build from sources:
Read [how to build & install the game]({{ site.baseurl }}/build/).
