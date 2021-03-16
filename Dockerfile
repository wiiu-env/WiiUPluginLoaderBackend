FROM wiiuenv/devkitppc:20210101

COPY --from=wiiuenv/wiiumodulesystem:20210313 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20210316 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210109 /artifacts $DEVKITPRO

WORKDIR project