#include "tradevault/TradeError.hpp"
#include <cmath>


//Error Messages 
//Using a switch for trade errors as we are comparing fixed required constants as opposed to using different functions                    
//A switch without a default was also used to ensure that the compiler warns if the error enum grows and this function stops covering it    
//Avoids embedding strings throughout the code                                                                     
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

        case TradeError::InvalidTimestamp:     //Keep temporarily 
            return "Trade timestamp is invalid.";

        case TradeError::TradeNotFound:
            return "Trade could not be found.";

        case TradeError::TradeAlreadyCancelled:
            return "Trade is already cancelled.";

        case TradeError::InternalError:     //Useful Error once PostgreSQL is implimented 
            return "An internal error occurred.";
    }

    return "Unknown trade error.";
}

//Validation Logic 
bool TradeValidator::isValidQuantity(int quantity) { return quantity > 0; }

//Use isfinite in orderr to make sure we are not getting an infinite value 
bool TradeValidator::isValidPrice(double price) { return std::isfinite(price) && price > 0.0; }

bool TradeValidator::isValidInstrument(const std::string& instrument) {
    return !instrument.empty();
}

bool TradeValidator::isValidCounterparty(const std::string& counterparty) {
    return !counterparty.empty();
}