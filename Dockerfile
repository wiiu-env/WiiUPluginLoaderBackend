FROM wiiuenv/devkitppc:20210920

COPY --from=wiiuenv/wiiumodulesystem:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20210924 /artifacts $DEVKITPRO

WORKDIR project