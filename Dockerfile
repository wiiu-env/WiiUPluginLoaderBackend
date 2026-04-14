FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:reentfix-dev-20260414-cd66e5f /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:abifix-dev-20260414-a43b43c /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260331  /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libbuttoncombo:20260331 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libiopshell:20260318 /artifacts $DEVKITPRO

WORKDIR project
