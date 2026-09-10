# TradeVault

[CI](https://github.com/alankhal/tradevault/actions/workflows/ci.yml/badge.svg)](https://github.com/alankhal/tradevault/actions/workflows/ci.yml)

A trade-capture microservice written in C++20. TradeVault will accept trades over a REST API,
validate them, persist them to PostgreSQL, and publish trade events to a Kafka-compatible
stream, with the observability, load testing, containerization, and deployment automation
a production service needs.

**Status:** M0 complete. The build system, dependency management, unit tests, and CI on
Linux and Windows are in place. Next is the domain model (M1). 
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

| Path | Contents |
|---|---|
| `src/include/tradevault/` | Public headers |
| `src/core/` | Domain library (`tradevault_core`) |
| `src/main.cpp` | Service entry point (`tradevault_api`) |
| `tests/` | GoogleTest suites (`core_tests`) |
| `.github/workflows/` | CI pipeline |

## CI

Every pull request and every push to `main` runs four checks: clang-format, build and test
on Ubuntu 24.04 (GCC) and Windows (MSVC), and clang-tidy. `main` is protected, so nothing
merges unless all four pass.

## Roadmap

| Milestone | Scope | Status |
|---|---|---|
| M0 | Repository scaffold, toolchain, CI | ✅ Done |
| M1 | Domain model and in-memory service | Next |
| M2 | REST API (Drogon) | |
| M3 | PostgreSQL | |
| M4 | Docker | |
| M5 | Kafka/events | |
| M6 | Observability | |
| M7 | Load testing | |
| M8 | Terraform/cloud | |
