#include "api/HttpErrorMapper.hpp"

drogon::HttpStatusCode HttpErrorMapper::statusFor(TradeError error)
{
    switch (error)
    {
    //Grouped for missing/incorrect parameters
    case TradeError::InvalidQuantity:
    case TradeError::InvalidPrice:
    case TradeError::InvalidInstrument:
    case TradeError::MissingCounterparty:
    case TradeError::InvalidTimestamp:
        return drogon::k400BadRequest;
    
    //Missing Trade
    case TradeError::TradeNotFound:
        return drogon::k404NotFound;

    //Repeated trade calls or calling a cancelled trade 
    case TradeError::TradeAlreadyExists:
    case TradeError::TradeAlreadyCancelled:
        return drogon::k409Conflict;

    //Unexpected Error
    case TradeError::InternalError:
        return drogon::k500InternalServerError;
    }

    //Issue with server itself
    return drogon::k500InternalServerError;
}

drogon::HttpResponsePtr HttpErrorMapper::toResponse(TradeError error)
{
    auto status = statusFor(error);

    Json::Value body;
    body["type"] = "about:blank";
    body["status"] = static_cast<int>(status);
    body["detail"] = tradeErrorMessage(error);

    switch (status)
    {
    case drogon::k400BadRequest:
        body["title"] = "Bad Request";
        break;

    case drogon::k404NotFound:
        body["title"] = "Not Found";
        break;

    case drogon::k409Conflict:
        body["title"] = "Conflict";
        break;

    default:
        body["title"] = "Internal Server Error";
        break;
    }

    auto response = drogon::HttpResponse::newHttpJsonResponse(body); 
    response->setStatusCode(status); 

    return response;
}