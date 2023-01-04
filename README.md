[![CI-Release](https://github.com/wiiu-env/WiiUPluginLoaderBackend/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/WiiUPluginLoaderBackend/actions/workflows/ci.yml)

# Wii U Plugin Loader Backend
This is the backend for the [WiiUPluginSystem](https://github.com/wiiu-env/WiiUPluginSystem). Check out the readme for more information about the Plugin System.

## Usage
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `PluginBackend.wms` into `sd:/wiiu/environments/[ENVIRONMENT]/modules`.  
2. Requires the [WUMSLoader](https://github.com/wiiu-env/WUMSLoader) in `sd:/wiiu/environments/[ENVIRONMENT]/modules/setup`.
3. Requires the [FunctionPatcherModule](https://github.com/wiiu-env/FunctionPatcherModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
4. Requires the [MemoryMappingModule](https://github.com/wiiu-env/MemoryMappingModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

Plugins needs to be placed into the following directory:

```
sd:/wiiu/environments/[ENVIRONMENT]/plugins
```

## Building
In order to be able to compile this, you need to have devkitPPC installed
[devkitPPC](https://devkitpro.org/wiki/Getting_Started) with the following
pacman packages installed.

```
(sudo) (dkp-)pacman -Syu --needed wiiu-dev
```

Make sure the following environment variables are set:
```
DEVKITPRO=/opt/devkitpro
DEVKITPPC=/opt/devkitpro/devkitPPC
```

Also make sure to install [wut](https://github.com/decaf-emu/wut), [WiiUPluginSystem](https://github.com/wiiu-env/WiiUPluginSystem), [WiiUModuleSystem](https://github.com/wiiu-env/WiiUModuleSystem), [libfunctionpatcher](https://github.com/wiiu-env/libfunctionpatcher) and [libmappedmemory](https://github.com/wiiu-env/libmappedmemory).

## Buildflags

### Logging
Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

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

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./source  --exclude ./source/elfio -i`

# Credits
- Maschell
- orboditilt
- https://github.com/serge1/ELFIO