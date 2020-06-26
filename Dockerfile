FROM wiiuenv/devkitppc:20200625

COPY --from=wiiuenv/wiiumodulesystem:20200626 /artifacts $DEVKITPRO
COPY --from=wiiuenv/wiiupluginsystem:20200626 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libfunctionpatcher:20200626 /artifacts $DEVKITPRO

WORKDIR project