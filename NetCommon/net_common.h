#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <deque>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <memory>

#define ASIO_STANDALONE
// #define QUEUE_MSGS_IMPLEMENTATION

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
