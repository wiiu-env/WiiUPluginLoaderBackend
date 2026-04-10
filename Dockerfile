FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:reentfix-dev-20260410-ae8bf4a /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:abifix-dev-20260410-b1ab874 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260331  /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libbuttoncombo:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libiopshell:20260318 /artifacts $DEVKITPRO

WORKDIR project
