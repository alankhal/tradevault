#include <spdlog/spdlog.h>

#include "tradevault/version.hpp"

int main() {
    spdlog::set_pattern(R"({"ts":"%Y-%m-%dT%H:%M:%S.%eZ","level":"%l","msg":"%v"})");
    spdlog::info("TradeVault starting, version {}", tradevault::version());
    return 0;
}