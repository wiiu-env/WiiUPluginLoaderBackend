[![CI-Release](https://github.com/wiiu-env/WiiUPluginLoaderBackend/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/WiiUPluginLoaderBackend/actions/workflows/ci.yml)

# Wii U Plugin Loader Backend
This is the Backend for the [WiiUPluginSystem](https://github.com/Maschell/WiiUPluginSystem). Check out the readme for more information about the Plugin System.

## Usage
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `PluginBackend.wms` into `sd:/wiiu/environments/[ENVIRONMENT]/modules`.  
2. Requires the [WUMSLoader](https://github.com/wiiu-env/WUMSLoader) in `sd:/wiiu/environments/[ENVIRONMENT]/modules/setup`.

Plugins needs to be placed into the following folder:

```
sd:/wiiu/environments/[ENVIRONMENT]/plugins
```

## Building
In order to be able to compile this, you need to have devkitPPC installed
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

Also make sure to install [wut](https://github.com/decaf-emu/wut), [WiiUPluginSystem](https://github.com/wiiu-env/WiiUPluginSystem) and the [WiiUModuleSystem](https://github.com/wiiu-env/WiiUModuleSystem).
It requires the [FunctionPatcherModule](https://github.com/wiiu-env/FunctionPatcherModule) to be running at the same time and it's linking aginst [libfunctionpatcher](https://github.com/wiiu-env/libfunctionpatcher).

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t wiiupluginloaderbackend-builder

# make 
docker run -it --rm -v ${PWD}:/project wiiupluginloaderbackend-builder make

# make clean
docker run -it --rm -v ${PWD}:/project wiiupluginloaderbackend-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./source -i`

# Credits
- Maschell
- orboditilt
- https://github.com/serge1/ELFIO