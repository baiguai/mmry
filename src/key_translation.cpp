#include "key_translation.h"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

std::string translateKey(void* eventPtr)
{
    std::string key_value = "";

#ifdef __linux__
    XEvent* event = (XEvent*)eventPtr;
    KeySym keysym;
    char buffer[10];
    XKeyEvent* keyEvent = (XKeyEvent*)event;

    XLookupString(keyEvent, buffer, sizeof(buffer), &keysym, nullptr);

    if (keysym == XK_D && (keyEvent->state & ShiftMask)) key_value = "D";
    if (keysym == XK_G && (keyEvent->state & ShiftMask)) key_value = "G";
    if (keysym == XK_g) key_value = "g";
    if (keysym == XK_h) key_value = "h";
    if (keysym == XK_i) key_value = "i";
    if (keysym == XK_j) key_value = "j";
    if (keysym == XK_k) key_value = "k";
    if (keysym == XK_M && (keyEvent->state & ShiftMask)) key_value = "M";
    if (keysym == XK_m) key_value = "m";
    if (keysym == XK_p) key_value = "p";
    if (keysym == XK_Q && (keyEvent->state & ShiftMask)) key_value = "Q";
    if (keysym == XK_Up) key_value = "UP";
    if (keysym == XK_Down) key_value = "DOWN";
    if (keysym == XK_Left) key_value = "LEFT";
    if (keysym == XK_Right) key_value = "RIGHT";
    if (keysym == XK_Home) key_value = "HOME";
    if (keysym == XK_End) key_value = "END";
    if (keysym == XK_Escape) key_value = "ESCAPE";
    if (keysym == XK_Return) key_value = "RETURN";
    if (keysym == XK_BackSpace) key_value = "BACKSPACE";
    if (keysym == XK_Delete) key_value = "DELETE";
    if (keysym == XK_space) key_value = "SPACE";
    if (keysym == XK_grave) key_value = "`";
    if (keysym == XK_apostrophe) key_value = "'";
    if (keysym == XK_colon) key_value = ":";
    if (keysym == XK_slash) key_value = "/";
    if (keysym == XK_question) key_value = "?";
#endif

#ifdef _WIN32
    MSG* msg = (MSG*)eventPtr;
    BYTE keyboardState[256] = {0};
    GetKeyboardState(keyboardState);

    if (msg->wParam == 'D' && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = "D";
    if (msg->wParam == 'G')
    {
        if (GetKeyState(VK_SHIFT) & 0x8000) key_value = "G";
        else key_value = "g";
    }
    if (msg->wParam == 'H') key_value = "h";
    if (msg->wParam == 'I') key_value = "i";
    if (msg->wParam == 'J') key_value = "j";
    if (msg->wParam == 'K') key_value = "k";
    if (msg->wParam == 'M')
    {
        if (GetKeyState(VK_SHIFT) & 0x8000) key_value = "M";
        else key_value = "m";
    }
    if (msg->wParam == 'P') key_value = "p";
    if (msg->wParam == 'Q' && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = "Q";
    if (msg->wParam == VK_UP) key_value = "UP";
    if (msg->wParam == VK_DOWN) key_value = "DOWN";
    if (msg->wParam == VK_LEFT) key_value = "LEFT";
    if (msg->wParam == VK_RIGHT) key_value = "RIGHT";
    if (msg->wParam == VK_HOME) key_value = "HOME";
    if (msg->wParam == VK_END) key_value = "END";
    if (msg->wParam == VK_ESCAPE) key_value = "ESCAPE";
    if (msg->wParam == VK_RETURN) key_value = "RETURN";
    if (msg->wParam == VK_BACK) key_value = "BACKSPACE";
    if (msg->wParam == VK_DELETE) key_value = "DELETE";
    if (msg->wParam == VK_SPACE) key_value = "SPACE";
    if (msg->wParam == VK_OEM_3) key_value = "`";
    if (msg->wParam == VK_OEM_7) key_value = "'";
    if (msg->wParam == VK_OEM_1 && (GetKeyState(VK_SHIFT) & 0x8000)) key_value = ":";
    if (msg->wParam == VK_OEM_2) key_value = "/";
    if ((msg->wParam == VK_OEM_2 && (GetKeyState(VK_SHIFT) & 0x8000))) key_value = "?";
#endif

    return key_value;
}
