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

// #define QUEUE_MSGS_IMPLEMENTATION
#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
