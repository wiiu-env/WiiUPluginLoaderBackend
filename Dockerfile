FROM wiiuenv/devkitppc:20210414

COPY --from=wiiuenv/wiiumodulesystem:20210414 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20210417 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20210109 /artifacts $DEVKITPRO

WORKDIR project