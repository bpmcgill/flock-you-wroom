<#
.SYNOPSIS
    Analyzes dataset files to show their format and content
.EXAMPLE
    .\analyze-datasets.ps1
#>

param(
    [string]$DatasetsPath = "..\datasets"
)

Write-Host "`n=== Dataset File Analysis ===" -ForegroundColor Green
Write-Host "Analyzing files in: $DatasetsPath`n" -ForegroundColor Yellow

Get-ChildItem -Path $DatasetsPath -File | ForEach-Object {
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "File: $($_.Name)" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($_.Length / 1KB, 2)) KB" -ForegroundColor Gray
    
    $content = Get-Content $_.FullName
    Write-Host "Lines: $($content.Count)" -ForegroundColor Gray
    
    # Show first 5 lines
    Write-Host "`nFirst 5 lines:" -ForegroundColor Yellow
    $content | Select-Object -First 5 | ForEach-Object {
        $truncated = if ($_.Length -gt 100) { $_.Substring(0, 100) + "..." } else { $_ }
        Write-Host "  $truncated" -ForegroundColor DarkGray
    }
    
    # Detect format
    $firstLine = $content[0]
    if ($firstLine.StartsWith("{") -or $firstLine.StartsWith("[")) {
        Write-Host "`nFormat: JSON" -ForegroundColor Magenta
    }
    elseif ($firstLine -match ",") {
        Write-Host "`nFormat: CSV" -ForegroundColor Magenta
        Write-Host "Columns: $($firstLine -split ',' | ForEach-Object { $_.Trim() })" -ForegroundColor DarkGray
    }
    
    # Count MAC addresses
    $macCount = ($content | Select-String -Pattern "[0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}").Count
    Write-Host "MAC addresses found: $macCount" -ForegroundColor Green
    
    Write-Host ""
}

Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━`n" -ForegroundColor DarkGray