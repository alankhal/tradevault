#include "tradevault/TradeService.hpp"


TradeService::TradeService(TradeRepository& repository)
    : m_repository(repository)
{
}