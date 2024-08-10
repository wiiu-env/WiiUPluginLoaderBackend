#pragma once

#include <cstdint>

class Input {
public:
    //!Constructor
    Input() = default;

    //!Destructor
    virtual ~Input() = default;

    enum eButtons : uint32_t {
        BUTTON_NONE    = 0x0000,
        VPAD_TOUCH     = 0x80000000,
        BUTTON_STICK_L = 0x80000,
        BUTTON_STICK_R = 0x40000,
        BUTTON_Z       = 0x20000,
        BUTTON_C       = 0x10000,
        BUTTON_A       = 0x8000,
        BUTTON_B       = 0x4000,
        BUTTON_X       = 0x2000,
        BUTTON_Y       = 0x1000,
        BUTTON_1       = BUTTON_Y,
        BUTTON_2       = BUTTON_X,
        BUTTON_LEFT    = 0x0800,
        BUTTON_RIGHT   = 0x0400,
        BUTTON_UP      = 0x0200,
        BUTTON_DOWN    = 0x0100,
        BUTTON_ZL      = 0x0080,
        BUTTON_ZR      = 0x0040,
        BUTTON_L       = 0x0020,
        BUTTON_R       = 0x0010,
        BUTTON_PLUS    = 0x0008,
        BUTTON_MINUS   = 0x0004,
        BUTTON_HOME    = 0x0002,
        BUTTON_SYNC    = 0x0001,
        STICK_R_LEFT   = 0x04000000,
        STICK_R_RIGHT  = 0x02000000,
        STICK_R_UP     = 0x01000000,
        STICK_R_DOWN   = 0x00800000,
        STICK_L_LEFT   = 0x40000000,
        STICK_L_RIGHT  = 0x20000000,
        STICK_L_UP     = 0x10000000,
        STICK_L_DOWN   = 0x08000000
    };

#define ANY_BUTTON_MASK (Input::eButtons::BUTTON_L |     \
                         Input::eButtons::BUTTON_R |     \
                         Input::eButtons::BUTTON_PLUS |  \
                         Input::eButtons::BUTTON_MINUS | \
                         Input::eButtons::BUTTON_A |     \
                         Input::eButtons::BUTTON_B |     \
                         Input::eButtons::BUTTON_X |     \
                         Input::eButtons::BUTTON_Y |     \
                         Input::eButtons::BUTTON_DOWN |  \
                         Input::eButtons::BUTTON_LEFT |  \
                         Input::eButtons::BUTTON_RIGHT | \
                         Input::eButtons::BUTTON_UP)

    typedef struct {
        uint32_t buttons_h;
        uint32_t buttons_d;
        uint32_t buttons_r;
        bool validPointer;
        bool touched;
        float pointerAngle;
        int32_t x;
        int32_t y;
    } PadData;

    PadData data{};
    PadData lastData{};
};
