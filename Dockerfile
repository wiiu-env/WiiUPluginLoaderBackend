FROM ghcr.io/wiiu-env/devkitppc:20230621

COPY --from=ghcr.io/wiiu-env/wiiumodulesystem:20230719 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:0.7.2-dev-20231105-45d17c5 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libfunctionpatcher:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libwupsbackend:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20230621 /artifacts $DEVKITPRO

WORKDIR project
