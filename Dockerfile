FROM wiiuenv/devkitppc:20220917

COPY --from=wiiuenv/wiiumodulesystem:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220923 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20220904 /artifacts $DEVKITPRO

WORKDIR project
