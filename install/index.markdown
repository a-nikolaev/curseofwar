---
layout: page
title: How to install the game
---

# Packages for popular Linux distributions

* Arch Linux: [AUR stable](https://aur.archlinux.org/packages/curseofwar/) or [AUR git](https://aur.archlinux.org/packages/curseofwar-git/).
* CRUX: [port](http://crux.nu/portdb/?a=repo&q=doom).
* Debian: 
* Ubuntu: [PPA team](https://launchpad.net/~curseofwar). 

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
