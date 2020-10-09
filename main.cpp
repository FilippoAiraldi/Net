#include <iostream>
#include <net.h>

enum class MsgType : uint32_t
{
    Fire,
    Move
};

class CustomClient : public net::client_interface<MsgType>
{
public:
    void fireBullet(const float x, const float y)
    {
        net::message<MsgType> msg(MsgType::Fire);
        msg << x << y;
        send(msg);
    }
};

int main()
{
    net::message<int> msg(212645);
    msg << 2200;
    msg << 17;
    msg << "245";
    std::cout << msg << std::endl;

    return 0;
}
