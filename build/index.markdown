---
layout: page
title: Build and Install
---

Build from sources on Linux
===========================

Ncurses version:
    
    $ make

SDL version:
    
    $ make SDL=yes

Install ncurses version
-----------------------

To install, `make install` with `DESTDIR` defined:

    # make DESTDIR=/path install

It copies:

* the executable `curseofwar` to `$DESTDIR/usr/bin/`
* the manpage `curseofwar.6.gz` to `$DESTDIR/usr/share/man/man6/`
* the `changelog.gz` to `$DESTDIR/usr/share/doc/curseofwar/`

To uninstall:
    
    # make DESTDIR=/path uninstall

Install SDL version
-------------------

To install:

    # make SDL=yes DESTDIR=/path install

It copies:

* the executable `curseofwar-sdl` to `$DESTDIR/usr/bin/`
* the manpage `curseofwar-sdl.6.gz` to `$DESTDIR/usr/share/man/man6/`
* `images` to `$DESTDIR/usr/share/curseofwar`

The SDL executable is searching for the game data (directory `images`) in the
following places:

1. `./` (Your current directory)
2. `$XDG_DATA_HOME/curseofwar` (Defaults to `~/.local/share/curseofwar`)
3. `~/.curseofwar` (If possible, use `~/.local/share/curseofwar`)
4. `/usr/local/share/curseofwar`
5. `/usr/share/curseofwar`
6. `/usr/share/curseofwar-sdl`
7. `/usr/share/curseofwar-common`

Make sure that `images` is one of those folders.

You can make use of the variable `INSTALL_DATA` to install `images` to a specific
place in your filesystem, instead of `$(DESTDIR)/usr/share/curseofwar`:

    # make SDL=yes DESTDIR=/path INSTALL_DATA=/game/data/path install

Example, how to build and install everything
============================================

It installs both executables to `/usr/bin/`, and all game data to
`/usr/share/curseofwar/`

To build and install:

    $ make
    $ make SDL=yes
    # make DESTDIR=/ install
    # make SDL=yes DESTDIR=/ install
      
Then to uninsitall, if necessary:

    # make SDL=yes DESTDIR=/ uninstall
    # make DESTDIR=/ uninstall

`DESTDIR=/` can be omitted here, but it's kept for completeness.

Build from sources on Windows
=============================

You will need the SDL development libraries installed on your computer.

There are several possibilities. You can use **cmake-gui**, for example.

- Fill the `source code` field with the path to the root directory of curseofwar
- Fill the `binairies` field with the path of your build directory
- Press `[Configure]`
- If your SDL install is at a custom location, you will need to fill the appropriate fields and hit `[Configure]` again.
- Under windows, you will need to uncheck `CW_NCURSE_FRONTEND` and `CW_SDL_MULTIPLAYER`
- Press `[Generate]`
- A project file was created in the build directory. Use it to build the projects.

Note that executable will fail if they are not run from the directory containing `images/`.
