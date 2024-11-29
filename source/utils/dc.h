#pragma once

#include <coreinit/debug.h>
#include <coreinit/screen.h>

extern "C" uint32_t __OSPhysicalToEffectiveUncached(uint32_t);

inline uint32_t DCReadReg32(const OSScreenID screen, const uint32_t index) {
    if (OSIsECOMode()) {
        return 0;
    }
    const auto regs = reinterpret_cast<uint32_t *>(__OSPhysicalToEffectiveUncached(0xc200000));
    return regs[index + (screen * 0x200)];
}


inline void DCWriteReg32(const OSScreenID screen, const uint32_t index, const uint32_t val) {
    if (OSIsECOMode()) {
        return;
    }
    const auto regs                = reinterpret_cast<uint32_t *>(__OSPhysicalToEffectiveUncached(0xc200000));
    regs[index + (screen * 0x200)] = val;
}

//  https://www.x.org/docs/AMD/old/42589_rv630_rrg_1.01o.pdf (reg id in document / 4)
#define D1GRPH_ENABLE_REG  0x1840
#define D1GRPH_CONTROL_REG 0x1841
#define D1GRPH_PITCH_REG   0x1848
#define D1OVL_PITCH_REG    0x1866
#define D1GRPH_X_END_REG   0x184d
#define D1GRPH_Y_END_REG   0x184e

inline void SetDCPitchReg(const OSScreenID screen, const uint16_t pitch) {
    DCWriteReg32(screen, D1GRPH_PITCH_REG, pitch);
    DCWriteReg32(screen, D1OVL_PITCH_REG, pitch);
}