FROM wiiuenv/devkitppc:20220213

COPY --from=wiiuenv/wiiumodulesystem:20220127 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220123 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20220213 /artifacts $DEVKITPRO

WORKDIR project