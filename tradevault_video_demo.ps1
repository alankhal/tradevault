# TradeVault portfolio video: guided, one-window persistence demonstration.
# Save this file anywhere, then run it from PowerShell.
# Requires a working Docker Compose stack in C:\dev\tradevault.
# Creates one sample trade; does not erase the database volume.

$ErrorActionPreference = 'Stop'
Set-Location 'C:\dev\tradevault'
$base = 'http://127.0.0.1:8080'

function Show-Step([int]$number, [string]$title) {
    Clear-Host
    Write-Host 'TRADEVAULT  |  LIVE PERSISTENCE DEMO' -ForegroundColor Cyan
    Write-Host "STEP $number OF 6  |  $title" -ForegroundColor White
    Write-Host ('-' * 65)
    Write-Host ''
}

function Wait-Next {
    Write-Host ''
    [void](Read-Host 'Press ENTER when you are ready for the next screen')
}

function Assert-DockerSuccess([string]$description) {
    if ($LASTEXITCODE -ne 0) {
        throw "$description failed with exit code $LASTEXITCODE."
    }
}

# 1. Show Docker containers and the API health endpoint.
Show-Step 1 'Healthy containers + HTTP health check'
docker compose ps
Assert-DockerSuccess 'docker compose ps'
$health = Invoke-RestMethod "$base/healthz"
$health | Format-List | Out-Host
if ($health.status -ne 'ok') { throw 'API health check did not return status=ok.' }
Wait-Next

# 2. Submit and display a live POST /trades request.
Show-Step 2 'Create a new trade with POST /trades'
$body = @{
    instrument   = 'AAPL'
    counterparty = 'PortfolioDemo'
    side         = 'Buy'
    price        = 215.50
    quantity     = 10
} | ConvertTo-Json
Write-Host 'Submitting sample request:' -ForegroundColor DarkCyan
Write-Host $body
Write-Host ''
$created = Invoke-RestMethod -Uri "$base/trades" -Method Post -ContentType 'application/json' -Body $body
$tradeId = $created.id
if (-not $tradeId) { throw 'No trade ID was returned from POST /trades.' }
Write-Host 'Trade created successfully:' -ForegroundColor Green
$created | Format-List | Out-Host
Write-Host "REMEMBER THIS TRADE ID: $tradeId" -ForegroundColor Cyan
Wait-Next

# 3. Confirm the exact trade in PostgreSQL (no credentials printed).
Show-Step 3 'Verify the stored PostgreSQL row'
Write-Host "Looking up trade ID $tradeId directly in PostgreSQL..."
Write-Host ''
$sql = "SELECT trade_id, instrument, counterparty, side, status, price, quantity FROM trades WHERE trade_id = $tradeId;"
docker compose exec -T db psql -U postgres -d tradevault -c $sql
Assert-DockerSuccess 'PostgreSQL SELECT'
Wait-Next

# 4. Remove containers WITHOUT the -v flag: named database volume remains.
Show-Step 4 'Stop and remove containers (keep database volume)'
Write-Host "Original trade ID: $tradeId"
Write-Host 'Running: docker compose down'
Write-Host ''
docker compose down
Assert-DockerSuccess 'docker compose down'
Write-Host ''
Write-Host 'Containers removed. Named PostgreSQL volume retained.' -ForegroundColor Green
Wait-Next

# 5. Recreate services and confirm readiness.
Show-Step 5 'Restart containers without deleting stored data'
Write-Host "Original trade ID: $tradeId"
Write-Host 'Running: docker compose up -d --wait'
Write-Host ''
docker compose up -d --wait
Assert-DockerSuccess 'docker compose up'
Write-Host ''
docker compose ps
Assert-DockerSuccess 'docker compose ps'
Write-Host ''
$healthAfterRestart = Invoke-RestMethod "$base/healthz"
if ($healthAfterRestart.status -ne 'ok') { throw 'API health check failed after restart.' }
Write-Host 'API responds again after restart.' -ForegroundColor Green
Wait-Next

# 6. Retrieve and verify same record.
Show-Step 6 'Retrieve original trade and prove persistence'
Write-Host "GET $base/trades/$tradeId"
Write-Host ''
$retrieved = Invoke-RestMethod "$base/trades/$tradeId"
@(
    [pscustomobject]@{
        Snapshot = 'Before restart'
        ID = $created.id
        Instrument = $created.instrument
        Counterparty = $created.counterparty
        Side = $created.side
        Status = $created.status
        Price = $created.price
        Quantity = $created.quantity
    },
    [pscustomobject]@{
        Snapshot = 'After restart'
        ID = $retrieved.id
        Instrument = $retrieved.instrument
        Counterparty = $retrieved.counterparty
        Side = $retrieved.side
        Status = $retrieved.status
        Price = $retrieved.price
        Quantity = $retrieved.quantity
    }
) | Format-Table -AutoSize | Out-Host

$sameTrade = (
    $retrieved.id -eq $created.id -and
    $retrieved.instrument -eq $created.instrument -and
    $retrieved.counterparty -eq $created.counterparty -and
    $retrieved.side -eq $created.side -and
    $retrieved.status -eq $created.status -and
    $retrieved.price -eq $created.price -and
    $retrieved.quantity -eq $created.quantity
)

if (-not $sameTrade) { throw 'FAIL: Retrieved trade differs from the original.' }
Write-Host 'PASS: The same trade survived a full Docker Compose restart!' -ForegroundColor Green
Write-Host ''
Write-Host 'End of live demonstration. You may now move on to the source-code and CI/CD sections.'
[void](Read-Host 'Press ENTER to finish')
