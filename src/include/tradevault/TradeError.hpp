#pragma once

#include <string>

//List of potential errors that can occur when setting up trade  
enum class TradeError {
    InvalidQuantity,
    InvalidPrice,
    InvalidInstrument,
    MissingCounterparty,
    InvalidTimestamp,

    TradeNotFound,
    TradeAlreadyExists,
    TradeAlreadyCancelled,

    InternalError
};

namespace TradeValidator {
    bool isValidQuantity(int quantity);
    bool isValidPrice(double price);
    bool isValidInstrument(const std::string& instrument);
    bool isValidCounterparty(const std::string& counterparty);
}  

std::string tradeErrorMessage(TradeError error);