#pragma once

#include <json/json.h>

#include "tradevault/Trade.hpp"

namespace TradeJson
{
    Json::Value toJson(const Trade& trade);
}