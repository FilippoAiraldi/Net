// #define QUEUE_MSGS_IMPLEMENTATION

#include <iostream>
#include <net.h>
#include <uuids/uuid.h>

int main()
{
    uuids::uuid id = uuids::uuid::create();
    net::message<double> client;

    return 0;
}
