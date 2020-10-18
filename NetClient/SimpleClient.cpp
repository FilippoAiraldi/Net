#include <iostream>
#include <net.h>

#include <X11/Xlib.h>
#include "X11/keysym.h"
#include <unordered_map>

bool _internal_key_press(KeySym ks) // ks like XK_Shift_L, see /usr/include/X11/keysymdef.h
{
    Display *dpy = XOpenDisplay(":0");
    char keys_return[32];
    XQueryKeymap(dpy, keys_return);
    KeyCode kc2 = XKeysymToKeycode(dpy, ks);
    bool isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
    XCloseDisplay(dpy);
    return isPressed;
}

std::unordered_map<int, bool> keyStates = {{XK_q, false}, {XK_p, false}};

bool is_key_pressed(KeySym key)
{
    if (keyStates.find(key) == keyStates.end())
        return false;

    if (_internal_key_press(key))
    {
        bool pressed = !keyStates[key];
        keyStates[key] = true;
        return pressed;
    }
    keyStates[key] = false;
    return false;
}

class Client : public net::client_interface<net::msgType>
{
public:
    void pingServer()
    {
        net::message<net::msgType> msg;
        msg.header.id = net::msgType::ServerPing;

        auto timeNow = std::chrono::system_clock::now();
        msg << timeNow;

        this->send(msg);
    }
};

int main()
{
    Client c;
    c.connect("192.168.1.91", 60000);

    if (!c.isConnected())
    {
        std::cout << "Connection failed.\n";
        return 0;
    }

    std::cout << "q: quit.\n"
              << "p: ping.\n";

    while (1)
    {
        if (c.isConnected())
        {
            if (!c.getIncomingMessages().empty())
            {
                auto msg = c.getIncomingMessages().pop_front().msg;

                switch (msg.header.id)
                {
                case net::msgType::ServerPing:
                {
                    auto timeNow = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point timeThen;
                    msg >> timeThen;
                    std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
                }
                break;
                case net::msgType::ServerAccept:
                    break;
                case net::msgType::ServerDeny:
                    break;
                case net::msgType::ServerMessage:
                    break;
                case net::msgType::MessageAll:
                    break;
                }
            }
        }
        else
        {
            std::cout << "Server down\n";
            break;
        }

        if (is_key_pressed(XK_q))
            break;

        if (is_key_pressed(XK_p))
            c.pingServer();
    }

    std::cout << "Disconnected.\n";
    return 0;
}
