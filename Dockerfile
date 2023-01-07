FROM wiiuenv/devkitppc:20221228

COPY --from=wiiuenv/wiiumodulesystem:20230106 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20230106 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20220904 /artifacts $DEVKITPRO

WORKDIR project
