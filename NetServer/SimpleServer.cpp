#include <iostream>
#include <net.h>

class Server : public net::server_interface<net::msgType>
{
public:
    Server(uint16_t port) : net::server_interface<net::msgType>(port) {}

protected:
    virtual bool onClientConnecting(std::shared_ptr<net::connection<net::msgType>> client)
    {
        return true;
    }

    virtual void onClientDisconnected(std::shared_ptr<net::connection<net::msgType>> client)
    {
    }

    virtual void onMessage(std::shared_ptr<net::connection<net::msgType>> client, net::message<net::msgType> &msg)
    {
        switch (msg.header.id)
        {
        case net::msgType::ServerPing:
        {
            std::cout << "[CONNECTION " << client->getId() << "] Server ping.\n";
            this->sendMessage(client, msg); // bounce message back
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
};

int main()
{
    Server server(60000);
    server.start();

    while (1)
    {
        server.Update(-1, true);
    }

    return 0;
}
