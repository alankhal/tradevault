--PostgreSQL constratins needed to protect stored data 
CREATE TABLE trades
(
    trade_id       BIGINT PRIMARY KEY, --Requires primary key as each id is unique
    instrument     TEXT NOT NULL, --Requires NOTNULL as instrument is needed for trade 
    counterparty   TEXT NOT NULL,
    side           TEXT NOT NULL CHECK (side IN ('Buy', 'Sell')), --Since side is an enum within my C++ files, going to have to make buy and sell stored as text for simplicity 
    status         TEXT NOT NULL CHECK (status IN ('Booked', 'Cancelled')),
    price          NUMERIC(18,4) NOT NULL CHECK (price > 0), --Used Numeric in order to pin point floating point numbers 
    quantity       INTEGER NOT NULL CHECK (quantity > 0),
    timestamp      TIMESTAMPTZ NOT NULL
);