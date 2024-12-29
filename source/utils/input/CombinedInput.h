#pragma once
#include "Input.h"

class CombinedInput final : public Input {
public:
    void combine(const Input &b) {
        data.buttons_d |= b.data.buttons_d;
        data.buttons_r |= b.data.buttons_r;
        data.buttons_h |= b.data.buttons_h;
        if (!data.touched) {
            data.touched = b.data.touched;
        }
        if (!data.validPointer) {
            data.validPointer = b.data.validPointer;
            data.pointerAngle = b.data.pointerAngle;
            data.x            = b.data.x;
            data.y            = b.data.y;
        }
    }

    void reset() {
        data = {};
    }
};