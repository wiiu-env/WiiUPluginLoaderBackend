# Wii U Plugin Loader Backend
This is the Backend for the [WiiUPluginSystem](https://github.com/Maschell/WiiUPluginSystem). Check out the readme for more information about the Plugin System.

## Usage
Put the `hook_payload.elf` in the `sd:/wiiu` folder of your sd card and load it via the [SystemMenuHook](https://github.com/wiiu-env/SystemMenuHook).

Plugins needs to be placed into the following folder:

```
sd:/wiiu/plugins
```

To autoboot into plugins, place them into the following folder:
```
sd:/wiiu/autoboot_plugins
```


Plugins can be disabled/enabled using the [WiiUPluginSystemGUI](https://github.com/wiiu-env/WiiUPluginLoaderGUI).

## Building
In order to be able to compile this, you need to have installed
[devkitPPC](https://devkitpro.org/wiki/Getting_Started) with the following
pacman packages installed.

```
pacman -Syu devkitPPC
```

Make sure the following environment variables are set:
```
DEVKITPRO=/opt/devkitpro
DEVKITPPC=/opt/devkitpro/devkitPPC
```


# Credits
Maschell
orboditilt