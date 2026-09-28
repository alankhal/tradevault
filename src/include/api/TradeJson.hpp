#pragma once

#include <json/json.h>
#include "tradevault/Trade.hpp"

//Does the conversion for Trade to JSON, allowing for less repition and easier changes if needed 
namespace TradeJson
{
    Json::Value toJson(const Trade& trade);
}