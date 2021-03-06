# RpTogether-Server

Server for [RpTogether](#roleplay-together-project), a game for creating and playing classic paper roleplays online.

## Table of content

Table of Contents
=================

* [RpTogether-Server](#rptogether-server)
  * [Table of content](#table-of-content)
  * [Roleplay-Together project](#roleplay-together-project)
  * [Install](#install)
    * [Windows](#windows)
    * [Ubuntu (or Windows with MinGW)](#ubuntu-or-windows-with-mingw)
      * [Requirements](#requirements)
      * [Install steps](#install-steps)
  * [Run](#run)
  * [Special credits](#special-credits)

## Roleplay-Together project

User can create its own roleplay, save it, and share it online so every player can try it.

Roleplays are both easy to create and powerfully customizable to grant an unlimited game lifetime.

*More information about Client and roleplays editor will arrive as development progress.*

Get more details about gameplay on the [wiki](https://github.com/ThisALV/RpTogether-Server/wiki).

## Install

### Windows

*Installer coming soon...*

### Ubuntu (or Windows with MinGW)

#### Requirements

- git
- cmake >= 3.12
- g++ >= 7

Install with :

```shell
sudo apt-get install git cmake g++
```

#### Install steps

Open a terminal and run following commands :

```shell
git clone https://github.com/ThisALV/RpTogether-Server
cd RpTogether-Server/dist
sudo ./get-deps-debian.sh # or ./get-deps-msys2.sh <32|64> for Windows MinGW users depending on current system architecture
cd ..
./build.sh # add --mingw option if you're Windows MinGW user
sudo ./install.sh
```

If you don't want to install RpT-Server at system level, add `--local` option to `./build.sh`.
It will install files to ./dist/install directory.

## Run

```shell
rpt-server --game <game_name> # for Linux users
rpt-server.exe --game <game_name> # for Windows MinGW users

## If locally installed with --local
./dist/install/bin/rpt-server --game <game_name> # for Linux users
./dist/install/bin/rpt-server.exe --game <game_name> # for Windows MinGW users
```

## Special credits

Doxygen doc-style directory is [forked](https://github.com/ThisALV/doxygen-dark-theme) from [MaJerle repo](https://github.com/MaJerle/doxygen-dark-theme) adjusting some color settings like for menus or links.