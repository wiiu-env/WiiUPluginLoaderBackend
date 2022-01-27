FROM wiiuenv/devkitppc:20211229

COPY --from=wiiuenv/wiiumodulesystem:20220127 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220123 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20211001 /artifacts $DEVKITPRO

WORKDIR project