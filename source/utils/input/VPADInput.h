#pragma once
/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "Input.h"

#include <coreinit/thread.h>
#include <vpad/input.h>

class VPadInput final : public Input {
public:
    //!Constructor
    VPadInput() = default;

    //!Destructor
    ~VPadInput() override = default;

    bool update(const int32_t width, const int32_t height) {
        lastData = data;

        data      = {};
        vpadError = VPAD_READ_UNINITIALIZED;

        int maxAttempts = 100;
        do {
            if (VPADRead(VPAD_CHAN_0, &vpad, 1, &vpadError) > 0 && vpadError == VPAD_READ_SUCCESS) {
                data.buttons_r    = vpad.release;
                data.buttons_h    = vpad.hold;
                data.buttons_d    = vpad.trigger;
                data.validPointer = !vpad.tpNormal.validity;
                data.touched      = vpad.tpNormal.touched;

                VPADGetTPCalibratedPoint(VPAD_CHAN_0, &tpCalib, &vpad.tpFiltered1);

                //! calculate the screen offsets
                data.x = -(width >> 1) + (int32_t) (((float) tpCalib.x / 1280.0f) * (float) width);
                data.y = -(height >> 1) + (int32_t) (float) height - (((float) tpCalib.y / 720.0f) * (float) height);

                return true;
            } else {
                OSSleepTicks(OSMillisecondsToTicks(1));
            }
        } while (--maxAttempts > 0 && vpadError == VPAD_READ_NO_SAMPLES);
        return false;
    }

    VPADStatus vpad         = {};
    VPADReadError vpadError = {};
    VPADTouchData tpCalib   = {};
};