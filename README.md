[Nightly builds](https://github.com/Maschell/WiiUPluginLoader/releases) | [Issue Tracker](https://github.com/Maschell/WiiUPluginLoader/issues) | [Discussion](https://gbatemp.net/threads/wii-u-plugin-system.496659/) | [Discord](https://discord.gg/bZ2rep2) | [Wiki](https://maschell.github.io/WiiUPluginSystem/dev_overview.html)

# Wii U Plugin Loader [![Build Status](https://api.travis-ci.org/Maschell/WiiUPluginLoader.svg?branch=master)](https://travis-ci.org/Maschell/WiiUPluginLoader)

This is the plugin loader for the [WiiUPluginSystem](https://github.com/Maschell/WiiUPluginSystem).

# Features

The project is still in it's very early days, but it already has basic functions to play with.

- The plugins will be loaded, even when you swap the running game.
- Support for up to 32 plugins at the same time.
- Each plugin can override up to 100 different existing system functions.
- Multiple plugins can override the same system functions.
- Plugins can register for certain hook (for example whenever an application was started)
- Plugins inherit the SD/USB access from the loader. All plugins have global SD and USB (FAT32 only) access.
- Plugins can be configured at run-time. Press L, DPAD down, and minus on the gamepad at the same time to open the configuration menu.

## Usage

Use the "Wii U Plugin Loader" to load plugins from the sd card. It is built to be loaded through the homebrew launcher, which can be either loaded with the browser exploit or haxchi.
Plugins needs to be placed into the following folder:

```
sd:/wiiu/plugins
```

You need to start this Application every time you reboot your console.
When you re-enter the homebrew launcher, the plugins will get unloaded.
This means it's not possible to combine this with other homebrews (yet).

## Create plugins
Information on how to create plugin can be found in the [wiki](https://maschell.github.io/WiiUPluginSystem/dev_plugin_creation_overview.html).

## Building
Make sure the toolchain is uptodate `pacman -Syu devkitPPC devkitARM vim general-tools`

For building you need: 
- [libiosuhax](https://github.com/dimok789/libiosuhax) (Build WITHOUT the WUT flag set.)
- [libfat](https://github.com/Maschell/libfat/) (Build with `make wiiu-release && make wiiu-install`)
- [dynamic_libs](https://github.com/Maschell/dynamic_libs/tree/lib) for access to the functions.
- [libutils](https://github.com/Maschell/libutils) for common functions.  
- [libgui](https://github.com/Maschell/libgui) for the gui elements.

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

A detailed instruction can be found in the Wiki:

- [How to compile the loader](https://maschell.github.io/WiiUPluginSystem/dev_compile_loader.html)  

### Building using the Dockerfile
It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once
docker build . -t wups-loader-builder

# make 
docker run -it --rm -v ${PWD}:/project wups-loader-builder make

# make clean
docker run -it --rm -v ${PWD}:/project wups-loader-builder make clean
```

# Load a plugin via network

While the loader is running, it's possible to load a single plugin via [wiiload](http://wiibrew.org/wiki/Wiiload).  
When using this feature, this and all plugins in `sd:/wiiu/plugins/temp` will be loaded. The plugin will copied to the SDCard, this mean a SDCard is required.
A windows executable can be found in `tools/wiiload.exe`  
More information about wiiload and alternatives can be found here: http://wiibrew.org/wiki/Wiiload

# Credits
Some files are based on brainslug by Chadderz:  
https://github.com/Chadderz121/brainslug-wii  
Much stuff also wouldn't be possible without dimok789. He made many great tools and homebrew this stuff in based on (Makefiles, Mocha, homebrew channel, udp logger, dynamic_libs etc.)  
Also thanks to everyone who made actual exploits.  