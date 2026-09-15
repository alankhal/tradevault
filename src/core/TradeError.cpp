#include "tradevault/TradeError.hpp"
#include <cmath>


//Error Messages 
//Using a switch for trade errors as we are comparing fixed required constants as opposed to using different functions
//A switch without a default was also used to ensure that the compiler warns if the error enum grows and this function stops covering it
std::string tradeErrorMessage(TradeError error) {
    switch (error) {
        case TradeError::InvalidQuantity:
            return "Trade quantity must be greater than zero.";

        case TradeError::InvalidPrice:
            return "Trade price must be greater than zero.";

        case TradeError::InvalidInstrument:
            return "Trade instrument is invalid.";

        case TradeError::MissingCounterparty:
            return "Trade must contain a counterparty.";

        case TradeError::InvalidTimestamp:
            return "Trade timestamp is invalid.";

        case TradeError::TradeNotFound:
            return "Trade could not be found.";

        case TradeError::TradeAlreadyExists:
            return "Trade already exists.";

        case TradeError::TradeAlreadyCancelled:
            return "Trade is already cancelled.";

        case TradeError::InternalError:
            return "An internal error occurred.";
    }

    return "Unknown trade error.";
}

//Validation Logic 
bool TradeError::isValidQuantity(int quantity) { return quantity > 0; }

//Use 
bool TradeError::isValidPrice(double price) { return std::isfinite(price) && price > 0.0; }

bool TradeError::isValidInstrument(const std::string& instrument) {
    return !instrument.empty();
}

bool TradeError::isValidCounterparty(const std::string& counterparty) {
    return !counterparty.empty();
}