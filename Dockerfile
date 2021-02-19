FROM wiiuenv/devkitppc:20210101

COPY --from=wiiuenv/wiiumodulesystem:20210219 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20210109 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210109 /artifacts $DEVKITPRO

WORKDIR project