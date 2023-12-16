#pragma once
#include "Input.h"
class CombinedInput : public Input {
public:
    void combine(const Input &b) {
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

    void process() {
        data.buttons_d |= (data.buttons_h & (~lastData.buttons_h));
        data.buttons_r |= (lastData.buttons_h & (~data.buttons_h));
        lastData.buttons_h = data.buttons_h;
    }

    void reset() {
        data = {};
    }
};