FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:20260225 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260225 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20260208  /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20260131 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20260131 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260131 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libbuttoncombo:20260112 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libiopshell:20260318 /artifacts $DEVKITPRO

WORKDIR project
