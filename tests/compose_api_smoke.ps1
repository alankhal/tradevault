
$ErrorActionPreference = "Stop"
$baseUrl = "http://127.0.0.1:8080"

# Test 1: Verify the API is healthy
$health = Invoke-RestMethod "$baseUrl/healthz"

if ($health.status -ne "ok") {
    throw "Health check failed"
}

Write-Host "PASS: GET /healthz"

# Test 2: Create a trade
$body = @{
    instrument = "AAPL"
    counterparty = "ComposeSmokeTest"
    side = "Buy"
    price = 123.45
    quantity = 2
} | ConvertTo-Json

$created = Invoke-RestMethod -Uri "$baseUrl/trades" -Method Post -ContentType "application/json" -Body $body

if (-not $created.id) {
    throw "Trade creation failed"
}

Write-Host "PASS: POST /trades"

# Test 3: Retrieve the created trade
$trade = Invoke-RestMethod "$baseUrl/trades/$($created.id)"

if ($trade.instrument -ne "AAPL" -or $trade.quantity -ne 2) {
    throw "Retrieved trade does not match"
}

Write-Host "PASS: GET /trades/$($created.id)"
