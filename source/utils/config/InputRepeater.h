#pragma once
#include "utils/input/Input.h"
#include <cstdint>

class InputRepeater {
public:
    // thresholdDelay: frames to wait before starting repeat (e.g., 15)
    // repeatRate: frames between repeats (e.g., 6)
    explicit InputRepeater(uint32_t thresholdDelay = 12, uint32_t repeatRate = 3)
        : mThresholdDelay(thresholdDelay), mRepeatRate(repeatRate) {}

    uint32_t update(const Input &input, uint32_t mask) {
        uint32_t currentPressed = input.data.buttons_d & mask;
        uint32_t currentHeld    = input.data.buttons_h & mask;

        if (currentPressed) {
            // New press: reset and return the button immediately
            mScrollTimer  = 0;
            mHeldButton   = currentPressed;
            mInRepeatMode = false;
            return currentPressed;
        }

        if (currentHeld && (currentHeld == mHeldButton)) {
            mScrollTimer++;
            uint32_t threshold = mInRepeatMode ? mRepeatRate : mThresholdDelay;

            if (mScrollTimer >= threshold) {
                mScrollTimer  = 0;
                mInRepeatMode = true;
                return currentHeld;
            }
        } else {
            // Button released or changed
            reset();
        }

        return 0; // No action this frame
    }

    void reset() {
        mScrollTimer  = 0;
        mHeldButton   = 0;
        mInRepeatMode = false;
    }

private:
    uint32_t mThresholdDelay;
    uint32_t mRepeatRate;
    uint32_t mScrollTimer = 0;
    uint32_t mHeldButton  = 0;
    bool mInRepeatMode    = false;
};