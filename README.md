# TradeVault

[![CI](https://github.com/alankhal/tradevault/actions/workflows/ci.yml/badge.svg)](https://github.com/alankhal/tradevault/actions/workflows/ci.yml)

A trade-capture microservice written in C++20. It accepts trades over a REST API, validates
them, stores them behind a swappable repository, and reports every failure as a typed error
mapped to a standard JSON problem response. It is built the way a production service is
built: tested, statically analysed, and CI-gated from the first commit, then grown one
milestone at a time into a service backed by PostgreSQL and an event stream.

**Status:** M0–M2 complete (toolchain, CI, domain model, in-memory service, REST API with
OpenAPI docs, 45 GoogleTest tests). **Next:** M3, PostgreSQL persistence.

## What works today

- A Drogon HTTP server exposing create, get, list, and cancel endpoints for trades, plus a
  `/healthz` health check.
- Validation on every create, with a typed error for each failure: a non-positive or
  non-finite price, a non-positive quantity, a missing instrument or counterparty, an unknown
  side, an unknown trade, or a trade that is already cancelled.
- Every error returned as an [RFC 7807](https://datatracker.ietf.org/doc/html/rfc7807)-style
  problem body (`type`, `title`, `status`, `detail`) with the matching HTTP status code.
- An OpenAPI 3.0 contract (`public/openapi.json`) served alongside a Swagger UI page, so the
  API can be explored and called from the browser.
- 45 GoogleTest tests across the validator, repository, service, and controller layers.
- CI on every pull request and push to `main`: build and test on Ubuntu 24.04 (GCC) and
  Windows (MSVC), plus clang-tidy static analysis.

## API

| Method | Route                  | Success         | Errors             | Purpose           |
| ------ | ---------------------- | --------------- | ------------------ | ----------------- |
| GET    | `/healthz`             | `200 OK`        |                    | Health check      |
| POST   | `/trades`              | `201 Created`   | `400`              | Capture a trade   |
| GET    | `/trades`              | `200 OK`        |                    | List all trades   |
| GET    | `/trades/{id}`         | `200 OK`        | `404`              | Fetch one trade   |
| POST   | `/trades/{id}/cancel`  | `200 OK`        | `404`, `409`       | Cancel a trade    |

With the server running, the interactive docs are at
<http://127.0.0.1:8080/swagger/index.html> and the raw spec at
<http://127.0.0.1:8080/openapi.json>.

### Example

Create a trade:

```bash
curl -i -X POST http://127.0.0.1:8080/trades \
  -H "Content-Type: application/json" \
  -d '{"instrument":"AAPL","counterparty":"Goldman Sachs","side":"Buy","price":215.5,"quantity":10}'
```

```json
HTTP/1.1 201 Created

{
  "id": 1,
  "instrument": "AAPL",
  "counterparty": "Goldman Sachs",
  "side": "Buy",
  "status": "Booked",
  "price": 215.5,
  "quantity": 10
}
```

Cancel it twice, and the second call is rejected:

```bash
curl -i -X POST http://127.0.0.1:8080/trades/1/cancel   # 200, status "Cancelled"
curl -i -X POST http://127.0.0.1:8080/trades/1/cancel   # 409
```

```json
HTTP/1.1 409 Conflict

{
  "type": "about:blank",
  "title": "Conflict",
  "status": 409,
  "detail": "Trade is already cancelled."
}
```

### Error mapping

| Domain error                                                                   | HTTP status |
| ------------------------------------------------------------------------------ | ----------- |
| `InvalidQuantity`, `InvalidPrice`, `InvalidInstrument`, `MissingCounterparty` | `400`       |
| Malformed JSON body, or `side` not `Buy`/`Sell`                               | `400`       |
| `TradeNotFound`                                                                | `404`       |
| `TradeAlreadyExists`, `TradeAlreadyCancelled`                                  | `409`       |
| `InternalError`                                                                | `500`       |

## Architecture

```
HTTP request
    │
    ▼
TradeController      (src/api)   parses JSON, calls the service, builds the HTTP response
    │   uses TradeJson (Trade → JSON) and HttpErrorMapper (TradeError → status + problem body)
    ▼
TradeService         (src/core)  validates input, applies business rules, returns Result<T, E>
    │
    ▼
TradeRepository      (interface)
    └── InMemoryTradeRepository  today; PostgreSQL in M3
```

Each layer receives the one below it through its constructor, and `app/main.cpp` wires them
together: repository → service → controller → Drogon.

## Design decisions

**Storage sits behind an interface.** The service receives an abstract `TradeRepository`
through its constructor. Today that is an in-memory store; in M3 a PostgreSQL implementation
takes its place without any change to business logic or the HTTP layer, and the in-memory
version stays available for fast tests.

**Failures are returned, not thrown.** Service calls return `Result<T, E>`, a small class
template that holds either a value or a typed error, so callers must handle the failure case
and every failure says exactly what went wrong. C++23's `std::expected` does the same job;
this project targets C++20, so `Result` fills that gap and can be swapped for
`std::expected` later.

**One place decides what an error looks like on the wire.** `HttpErrorMapper` is the only
code that turns a `TradeError` into a status code and problem body. The controller never
picks a status code for a domain failure itself, so adding a new error means updating one
switch, and the compiler warns if a case is missed.

**The contract is documented, not implied.** The OpenAPI spec describes every route, request
body, response schema, and error response, and is served by the same process as the API.

**Quality gates came first.** Static analysis and two compilers ran in CI before any feature
existed, so the codebase has never needed retrofitting to pass them.

## Quickstart

Prerequisites: a C++20 compiler (MSVC from Visual Studio 2022+, or GCC 13+), CMake 3.25+,
Ninja, and [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set. On Windows,
run these from **Developer PowerShell for VS** so the compiler is on `PATH`.

```
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The first configure takes several minutes while vcpkg builds the dependencies (Drogon,
spdlog, GoogleTest).

Run the service **from the repository root**, since it loads `config/config.json` and serves
`public/` relative to the working directory:

```
./build/debug/src/tradevault_api        # Linux
.\build\debug\src\tradevault_api.exe    # Windows
```

The server listens on `127.0.0.1:8080`. Change the address or port in `config/config.json`.

## Tests

| Suite        | Target       | Tests | Covers                                                        |
| ------------ | ------------ | ----- | ------------------------------------------------------------- |
| Validator    | `core_tests` | 12    | Quantity, price (incl. zero, negative, infinity, NaN), fields |
| Repository   | `core_tests` | 6     | Store, find, list, update, binary-search boundaries           |
| Service      | `core_tests` | 14    | Create, get, list, cancel, every validation failure           |
| Version      | `core_tests` | 1     | Semantic version string                                       |
| Controller   | `api_tests`  | 12    | Status codes and JSON bodies for every endpoint and error     |

The controller tests call each handler directly with a constructed `HttpRequest` and inspect
the response, so they run fast and need no open port.

## Repository layout

| Path                      | Contents                                                  |
| ------------------------- | --------------------------------------------------------- |
| `app/main.cpp`            | Server entry point: wires dependencies, starts Drogon     |
| `src/include/tradevault/` | Domain headers (`Trade`, `TradeService`, `Result`, …)     |
| `src/include/api/`        | HTTP-layer headers                                        |
| `src/core/`               | Domain library (`tradevault_core`)                        |
| `src/api/`                | Controller, JSON serialisation, error mapping             |
| `config/config.json`      | Drogon listener and static-file settings                  |
| `public/`                 | OpenAPI spec and Swagger UI                               |
| `tests/`                  | GoogleTest suites (`core_tests`, `api_tests`)             |
| `.github/workflows/`      | CI pipeline                                               |

## Roadmap

| Milestone | Scope                              | Status  |
| --------- | ---------------------------------- | ------- |
| M0        | Repository scaffold, toolchain, CI | ✅ Done |
| M1        | Domain model and in-memory service | ✅ Done |
| M2        | REST API (Drogon)                  | ✅ Done |
| M3        | PostgreSQL                         | ⏭️ Next |
| M4        | Docker                             |         |
| M5        | Kafka/events                       |         |
| M6        | Observability                      |         |
| M7        | Load testing                       |         |
| M8        | Terraform/cloud                    |         |

## Known limitations

- Trades live in memory and are lost when the server stops; M3 adds PostgreSQL.
- `GET /trades` returns every trade with no pagination yet.
- The clang-format CI check is paused until PostgreSQL is set up, and will be restored.