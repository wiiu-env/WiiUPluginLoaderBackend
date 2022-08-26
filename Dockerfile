FROM wiiuenv/devkitppc:20220806

COPY --from=wiiuenv/wiiumodulesystem:20220724 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20220826 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20220724 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220724 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libwupsbackend:20220724 /artifacts $DEVKITPRO

WORKDIR project