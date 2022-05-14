FROM wiiuenv/devkitppc:20220507

COPY --from=wiiuenv/wiiumodulesystem:20220512 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220513 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20220507 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220211 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20220514 /artifacts $DEVKITPRO

WORKDIR project