FROM wiiuenv/devkitppc:20211106

COPY --from=wiiuenv/wiiumodulesystem:20211207 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20211001 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20210924 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20211001 /artifacts $DEVKITPRO

WORKDIR project