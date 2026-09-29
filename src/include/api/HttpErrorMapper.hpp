#pragma once

#include <drogon/drogon.h>
#include "tradevault/TradeError.hpp"

namespace HttpErrorMapper
{
    //This will contain the mapping towards which error code it will contain
    drogon::HttpStatusCode statusFor(TradeError error);
    
    //Will hold the repetitive response construction that JSON uses 
    drogon::HttpResponsePtr toResponse(TradeError error);
} 