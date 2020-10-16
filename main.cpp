#include <iostream>
#include <uuids/uuid.h>

// #define QUEUE_MSGS_IMPLEMENTATION
#include <net.h>

int main()
{
    uuids::uuid id = uuids::uuid::create();

    std::deque<std::shared_ptr<net::message<int>>> dq;
    std::shared_ptr<net::message<int>> p;

    return 0;
}
