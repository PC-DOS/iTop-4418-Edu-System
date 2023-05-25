#include <linux/input.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "device.h"
#include "screen_ui.h"

const char *HEADERS[] = {
    "Power button short press to select",
    "Power button long press to menu on/off",
    "",
    NULL
};

const char *ITEMS[] = {
    "reboot system now",
    NULL
};

class droneUI : public ScreenRecoveryUI
{
public:
    droneUI() :
        mNextKeyIsLongPressed(false)
    {
    }

    virtual void NextCheckKeyIsLong(bool is_long_press) {
        //printf("%s: %d\n", __func__, is_long_press);
        mNextKeyIsLongPressed = is_long_press;
    }

    virtual KeyAction CheckKey(int key) {
        //printf("%s: key 0x%x", __func__, key);
        //if (IsKeyPressed(KEY_POWER)) {
        if (key == KEY_POWER) {
            if (mNextKeyIsLongPressed) {
                printf("UI Toggle\n");
                return TOGGLE;
            }
            return ENQUEUE;
        }
        return IGNORE;
    }

    void Init() {
         install_overlay_offset_x = 0;
         install_overlay_offset_y = 0;
         ScreenRecoveryUI::Init();
    }

private:
    bool mNextKeyIsLongPressed;
};

class droneDevice : public Device
{
public:
    droneDevice() :
        ui(new droneUI)
    {
    }

    RecoveryUI *GetUI() {
         return ui;
    }

    int HandleMenuKey(int keyCode, int visible) {
        if (visible) {
            switch (keyCode) {
            case KEY_POWER:
                return kInvokeItem;
            }
        }

        return kNoAction;
    }

    BuiltinAction InvokeMenuItem(int menuPosition) {
        switch (menuPosition) {
        case 0: return REBOOT;
        default: return NO_ACTION;
        }
    }

    const char* const* GetMenuHeaders() {
         return HEADERS;
    }

    const char* const* GetMenuItems() {
        return ITEMS;
    }

private:
    RecoveryUI *ui;
};

Device *make_device() {
     return new droneDevice;
}
