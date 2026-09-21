# TradeVault

[![CI](https://github.com/alankhal/tradevault/actions/workflows/ci.yml/badge.svg)](https://github.com/alankhal/tradevault/actions/workflows/ci.yml)

A trade-capture microservice written in C++20. It validates incoming trades, stores them
behind a swappable repository, and reports every failure as a typed error. It is built the
way a production service is built: tested, statically analysed, and CI-gated from the first
commit, then grown one milestone at a time into a REST service backed by PostgreSQL and an
event stream.

**Status:** M0–M1 complete (toolchain, CI, domain model, in-memory service, 32 unit tests).
**Now:** M2, the REST API (Drogon).

## What works today

- Create, get, and cancel trades through an in-memory service.
- Validation on every create, with a typed error for each failure: an invalid price, an
  unknown trade, or a trade that is already cancelled.
- 32 GoogleTest unit tests across the repository, service, and validation layers.
- CI on every pull request and push to `main`: clang-format, clang-tidy, and build + test on
  Ubuntu 24.04 (GCC) and Windows (MSVC). `main` is protected, so nothing merges unless all
  four checks pass.

## Design decisions

**Storage sits behind an interface.** The service receives an abstract repository through its
constructor. Today that is an in-memory store; in M3 a PostgreSQL implementation takes its
place without any change to business logic, and the in-memory version stays available for
fast unit tests.

**Failures are returned, not thrown.** Service calls return `Result<T, E>`, a small class
template that holds either a value or a typed error, so callers must handle the failure case
and every failure says exactly what went wrong. C++23's `std::expected` does the same job;
this project targets C++20, so `Result` fills that gap and can be swapped for
`std::expected` later.

**Quality gates came first.** Formatting, static analysis, and two compilers ran in CI before
any feature existed, so the codebase has never needed retrofitting to pass them.

## Quickstart

Prerequisites: a C++20 compiler (MSVC from Visual Studio 2022+, or GCC 13+), CMake 3.25+,
Ninja, and [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set. On Windows,
run these from **Developer PowerShell for VS** so the compiler is on `PATH`.

```
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The first configure takes a few minutes while vcpkg builds the dependencies.

Run the service:

```
./build/debug/src/tradevault_api        # Linux
.\build\debug\src\tradevault_api.exe    # Windows
```

## Repository layout

| Path                      | Contents                               |
| ------------------------- | -------------------------------------- |
| `src/include/tradevault/` | Public headers                         |
| `src/core/`               | Domain library (`tradevault_core`)     |
| `src/main.cpp`            | Service entry point (`tradevault_api`) |
| `tests/`                  | GoogleTest suites (`core_tests`)       |
| `.github/workflows/`      | CI pipeline                            |

## Planned API (M2)

| Method | Route                  | Purpose            |
| ------ | ---------------------- | ------------------ |
| POST   | `/trades`              | Capture a trade    |
| GET    | `/trades/{id}`         | Fetch a trade      |
| POST   | `/trades/{id}/cancel`  | Cancel a trade     |

## Roadmap

| Milestone | Scope                              | Status         |
| --------- | ---------------------------------- | -------------- |
| M0        | Repository scaffold, toolchain, CI | ✅ Done        |
| M1        | Domain model and in-memory service | ✅ Done        |
| M2        | REST API (Drogon)                  | 🚧 In progress |
| M3        | PostgreSQL                         |                |
| M4        | Docker                             |                |
| M5        | Kafka/events                       |                |
| M6        | Observability                      |                |
| M7        | Load testing                       |                |
| M8        | Terraform/cloud                    |                |
