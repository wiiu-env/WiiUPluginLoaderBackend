FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:reentfix-dev-20260403-5ca1144 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:abifix-dev-20260408-77ab748 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260331  /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libbuttoncombo:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libiopshell:20260318 /artifacts $DEVKITPRO

WORKDIR project
