<#
.SYNOPSIS
    Converts legacy dataset files to new database format for FlockYouWroom
.DESCRIPTION
    Scans the datasets folder for log files and converts them to the new
    CSV database format with GPS tracking and detection aggregation.
    Handles various CSV formats from Raven detectors and drone databases.
.PARAMETER DatasetsPath
    Path to the datasets folder (default: ..\datasets)
.PARAMETER OutputPath
    Path for the output database file (default: ..\datasets\detections.db)
.PARAMETER Merge
    Merge with existing database instead of overwriting
.EXAMPLE
    .\convert-datasets.ps1
.EXAMPLE
    .\convert-datasets.ps1 -DatasetsPath "C:\Data\drones" -OutputPath "C:\Output\merged.db" -Merge
#>

param(
    [string]$DatasetsPath = "..\datasets",
    [string]$OutputPath = "..\datasets\detections.db",
    [switch]$Merge,
    [switch]$ShowSample
)

# Device tracking hashtables
$devices = @{}
$unknownFormats = @()

function Detect-FileFormat {
    param([string]$filePath)
    
    $firstLine = Get-Content $filePath -First 1
    $secondLine = Get-Content $filePath -TotalCount 2 | Select-Object -Last 1
    
    # Check for JSON
    if ($firstLine.Trim().StartsWith("{") -or $firstLine.Trim().StartsWith("[")) {
        return "JSON"
    }
    
    # Check for CSV with headers
    if ($firstLine -match "mac|MAC|address|Address|SSID|ssid|Name|name") {
        # Detect specific CSV formats
        $headers = $firstLine.ToLower()
        
        if ($headers -match "manufacturer|oui") {
            return "RavenConfig"  # raven_configurations format
        }
        elseif ($headers -match "ssid.*mac.*channel") {
            return "WiFiScan"  # WiFi scan results
        }
        elseif ($headers -match "name.*mac.*type") {
            return "DeviceList"  # Device list format
        }
        else {
            return "CSVWithHeaders"
        }
    }
    
    # Check for CSV without headers (data starts immediately)
    if ($secondLine -match "^[0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}") {
        return "CSVNoHeaders"
    }
    
    return "Unknown"
}

function Parse-RavenConfig {
    param([string]$filePath)
    
    Write-Host "  Parsing as Raven Configuration JSON..." -ForegroundColor Gray
    
    try {
        $json = Get-Content $filePath -Raw | ConvertFrom-Json
        $count = 0
        
        foreach ($device in $json) {
            $mac = if ($device.mac) { $device.mac } 
                   elseif ($device.MAC) { $device.MAC }
                   elseif ($device.address) { $device.address }
                   else { continue }
            
            $detection = @{
                Success = $true
                MAC = $mac.ToUpper().Replace("-", ":")
                Type = if ($device.type) { $device.type } else { "Raven" }
                RSSI = 0  # Not available in config files
                Lat = 0.0
                Lon = 0.0
                Time = [DateTimeOffset]::Now.ToUnixTimeSeconds()
                Name = if ($device.name) { $device.name } else { "" }
                Manufacturer = if ($device.manufacturer) { $device.manufacturer } else { "" }
            }
            
            Add-Detection $detection
            $count++
        }
        
        Write-Host "    Imported $count devices from config" -ForegroundColor DarkGray
        return $count
    }
    catch {
        Write-Host "    Failed to parse JSON: $_" -ForegroundColor Red
        return 0
    }
}

function Parse-WiFiScanCSV {
    param([string]$filePath)
    
    Write-Host "  Parsing as WiFi Scan CSV..." -ForegroundColor Gray
    
    $lines = Get-Content $filePath
    $headers = $lines[0].ToLower() -split ','
    
    # Find column indices
    $macIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "mac|bssid|address" } | Select-Object -First 1))
    $ssidIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "ssid|name" } | Select-Object -First 1))
    $rssiIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "rssi|signal|strength" } | Select-Object -First 1))
    $channelIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "channel|ch" } | Select-Object -First 1))
    
    if ($macIdx -eq -1) {
        Write-Host "    No MAC column found" -ForegroundColor Red
        return 0
    }
    
    $count = 0
    for ($i = 1; $i -lt $lines.Count; $i++) {
        $parts = $lines[$i] -split ','
        
        if ($parts.Count -le $macIdx) { continue }
        
        $mac = $parts[$macIdx].Trim().Trim('"')
        if ([string]::IsNullOrWhiteSpace($mac) -or $mac -notmatch "^[0-9A-Fa-f]{2}[:\-]") {
            continue
        }
        
        $detection = @{
            Success = $true
            MAC = $mac.ToUpper().Replace("-", ":")
            Type = "WiFi"
            RSSI = if ($rssiIdx -ne -1 -and $parts.Count -gt $rssiIdx) { 
                try { [int]$parts[$rssiIdx] } catch { -70 }
            } else { -70 }
            Lat = 0.0
            Lon = 0.0
            Time = [DateTimeOffset]::Now.ToUnixTimeSeconds()
            SSID = if ($ssidIdx -ne -1 -and $parts.Count -gt $ssidIdx) { $parts[$ssidIdx].Trim().Trim('"') } else { "" }
        }
        
        Add-Detection $detection
        $count++
    }
    
    Write-Host "    Imported $count WiFi devices" -ForegroundColor DarkGray
    return $count
}

function Parse-DeviceListCSV {
    param([string]$filePath)
    
    Write-Host "  Parsing as Device List CSV..." -ForegroundColor Gray
    
    $lines = Get-Content $filePath
    $headers = $lines[0].ToLower() -split ','
    
    # Find column indices
    $macIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "mac|address" } | Select-Object -First 1))
    $nameIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "name|device" } | Select-Object -First 1))
    $typeIdx = [array]::IndexOf($headers, ($headers | Where-Object { $_ -match "type|category" } | Select-Object -First 1))
    
    if ($macIdx -eq -1) {
        Write-Host "    No MAC column found" -ForegroundColor Red
        return 0
    }
    
    $count = 0
    for ($i = 1; $i -lt $lines.Count; $i++) {
        $parts = $lines[$i] -split ','
        
        if ($parts.Count -le $macIdx) { continue }
        
        $mac = $parts[$macIdx].Trim().Trim('"')
        if ([string]::IsNullOrWhiteSpace($mac) -or $mac -notmatch "^[0-9A-Fa-f]{2}[:\-]") {
            continue
        }
        
        $detection = @{
            Success = $true
            MAC = $mac.ToUpper().Replace("-", ":")
            Type = if ($typeIdx -ne -1 -and $parts.Count -gt $typeIdx) { 
                $parts[$typeIdx].Trim().Trim('"') 
            } else { "BLE" }
            RSSI = 0
            Lat = 0.0
            Lon = 0.0
            Time = [DateTimeOffset]::Now.ToUnixTimeSeconds()
            Name = if ($nameIdx -ne -1 -and $parts.Count -gt $nameIdx) { $parts[$nameIdx].Trim().Trim('"') } else { "" }
        }
        
        Add-Detection $detection
        $count++
    }
    
    Write-Host "    Imported $count devices" -ForegroundColor DarkGray
    return $count
}

function Parse-GenericCSV {
    param([string]$filePath, [bool]$hasHeaders)
    
    Write-Host "  Parsing as generic CSV (headers: $hasHeaders)..." -ForegroundColor Gray
    
    $lines = Get-Content $filePath
    $startIdx = if ($hasHeaders) { 1 } else { 0 }
    $count = 0
    
    for ($i = $startIdx; $i -lt $lines.Count; $i++) {
        $line = $lines[$i].Trim()
        
        if ([string]::IsNullOrWhiteSpace($line) -or $line.StartsWith("#")) {
            continue
        }
        
        $parts = $line -split ','
        
        # Try to find MAC address (usually first column or recognizable pattern)
        $mac = $null
        $macIdx = -1
        
        for ($j = 0; $j -lt $parts.Count; $j++) {
            $part = $parts[$j].Trim().Trim('"')
            if ($part -match "^[0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}[:\-][0-9A-Fa-f]{2}$") {
                $mac = $part
                $macIdx = $j
                break
            }
        }
        
        if (-not $mac) { continue }
        
        # Try to extract other fields
        $type = "Unknown"
        $rssi = 0
        
        # Look for type indicators
        for ($j = 0; $j -lt $parts.Count; $j++) {
            $part = $parts[$j].Trim().Trim('"').ToLower()
            if ($part -match "ble|bluetooth") { $type = "BLE" }
            elseif ($part -match "wifi|802\.11") { $type = "WiFi" }
            elseif ($part -match "raven|drone") { $type = "Raven" }
        }
        
        # Look for RSSI (negative number)
        for ($j = 0; $j -lt $parts.Count; $j++) {
            if ($j -eq $macIdx) { continue }
            try {
                $val = [int]$parts[$j].Trim().Trim('"')
                if ($val -lt 0 -and $val -gt -120) {
                    $rssi = $val
                    break
                }
            }
            catch { }
        }
        
        $detection = @{
            Success = $true
            MAC = $mac.ToUpper().Replace("-", ":")
            Type = $type
            RSSI = $rssi
            Lat = 0.0
            Lon = 0.0
            Time = [DateTimeOffset]::Now.ToUnixTimeSeconds()
        }
        
        Add-Detection $detection
        $count++
    }
    
    Write-Host "    Imported $count devices" -ForegroundColor DarkGray
    return $count
}

function Parse-JsonLine {
    param([string]$line)
    
    try {
        $json = $line | ConvertFrom-Json
        return @{
            Success = $true
            MAC = if ($json.mac) { $json.mac.ToUpper() } else { $json.MAC.ToUpper() }
            Type = if ($json.type) { $json.type } else { "Unknown" }
            RSSI = if ($json.rssi) { $json.rssi } else { 0 }
            Lat = if ($json.lat) { $json.lat } elseif ($json.gps_latitude) { $json.gps_latitude } else { 0.0 }
            Lon = if ($json.lon) { $json.lon } elseif ($json.gps_longitude) { $json.gps_longitude } else { 0.0 }
            Time = if ($json.time) { $json.time } elseif ($json.timestamp) { $json.timestamp } else { [DateTimeOffset]::Now.ToUnixTimeSeconds() }
        }
    }
    catch {
        return @{ Success = $false }
    }
}

function Add-Detection {
    param($detection)
    
    $mac = $detection.MAC
    
    if (-not $devices.ContainsKey($mac)) {
        $devices[$mac] = @{
            MAC = $mac
            Type = $detection.Type
            RSSI = $detection.RSSI
            FirstSeen = $detection.Time
            LastSeen = $detection.Time
            Count = 1
            Locations = @()
            Name = if ($detection.Name) { $detection.Name } else { "" }
            SSID = if ($detection.SSID) { $detection.SSID } else { "" }
            Manufacturer = if ($detection.Manufacturer) { $detection.Manufacturer } else { "" }
        }
    }
    else {
        $device = $devices[$mac]
        $device.Count++
        
        # Update RSSI if we have a valid one
        if ($detection.RSSI -ne 0) {
            $device.RSSI = $detection.RSSI
        }
        
        # Update type if we have a more specific one
        if ($detection.Type -ne "Unknown" -and $device.Type -eq "Unknown") {
            $device.Type = $detection.Type
        }
        
        if ($detection.Time -lt $device.FirstSeen) {
            $device.FirstSeen = $detection.Time
        }
        if ($detection.Time -gt $device.LastSeen) {
            $device.LastSeen = $detection.Time
        }
        
        # Update name/SSID if we didn't have one
        if ($detection.Name -and -not $device.Name) {
            $device.Name = $detection.Name
        }
        if ($detection.SSID -and -not $device.SSID) {
            $device.SSID = $detection.SSID
        }
    }
    
    # Add location if valid
    if ($detection.Lat -ne 0.0 -and $detection.Lon -ne 0.0) {
        $location = "{0:F6},{1:F6}" -f $detection.Lat, $detection.Lon
        
        if ($devices[$mac].Locations -notcontains $location) {
            $devices[$mac].Locations += $location
        }
    }
}

function Process-File {
    param([string]$filePath)
    
    Write-Host "`nProcessing: $(Split-Path $filePath -Leaf)" -ForegroundColor Cyan
    
    $format = Detect-FileFormat $filePath
    Write-Host "  Detected format: $format" -ForegroundColor Gray
    
    $processedCount = 0
    
    switch ($format) {
        "JSON" {
            # Could be single JSON object or JSON lines
            $content = Get-Content $filePath -Raw
            if ($content.Trim().StartsWith("[")) {
                $processedCount = Parse-RavenConfig $filePath
            }
            else {
                # JSON lines format
                Get-Content $filePath | ForEach-Object {
                    $detection = Parse-JsonLine $_
                    if ($detection.Success) {
                        Add-Detection $detection
                        $processedCount++
                    }
                }
            }
        }
        "RavenConfig" {
            $processedCount = Parse-RavenConfig $filePath
        }
        "WiFiScan" {
            $processedCount = Parse-WiFiScanCSV $filePath
        }
        "DeviceList" {
            $processedCount = Parse-DeviceListCSV $filePath
        }
        "CSVWithHeaders" {
            $processedCount = Parse-GenericCSV $filePath $true
        }
        "CSVNoHeaders" {
            $processedCount = Parse-GenericCSV $filePath $false
        }
        default {
            Write-Host "  Unknown format, trying generic parser..." -ForegroundColor Yellow
            $processedCount = Parse-GenericCSV $filePath $false
            if ($processedCount -eq 0) {
                $unknownFormats += $filePath
            }
        }
    }
    
    return $processedCount
}

# Main script
Write-Host "`n=== FlockYouWroom Dataset Converter ===" -ForegroundColor Green
Write-Host "Datasets Path: $DatasetsPath" -ForegroundColor Yellow
Write-Host "Output Path: $OutputPath`n" -ForegroundColor Yellow

# Check if datasets folder exists
if (-not (Test-Path $DatasetsPath)) {
    Write-Host "ERROR: Datasets folder not found: $DatasetsPath" -ForegroundColor Red
    exit 1
}

# Show sample mode
if ($ShowSample) {
    Write-Host "=== SAMPLE MODE - Showing first few lines of each file ===`n" -ForegroundColor Magenta
    
    Get-ChildItem -Path $DatasetsPath -File | ForEach-Object {
        Write-Host "File: $($_.Name)" -ForegroundColor Cyan
        Write-Host "Format: $(Detect-FileFormat $_.FullName)" -ForegroundColor Gray
        Get-Content $_.FullName -TotalCount 3 | ForEach-Object {
            Write-Host "  $_" -ForegroundColor DarkGray
        }
        Write-Host ""
    }
    exit 0
}

# Load existing database if merging
if ($Merge -and (Test-Path $OutputPath)) {
    Write-Host "Loading existing database for merge..." -ForegroundColor Yellow
    
    Get-Content $OutputPath | ForEach-Object {
        $line = $_.Trim()
        
        if ([string]::IsNullOrWhiteSpace($line) -or $line.StartsWith("#")) {
            return
        }
        
        $parts = $line -split ','
        if ($parts.Count -ge 7) {
            $mac = $parts[0]
            $devices[$mac] = @{
                MAC = $mac
                Type = $parts[1]
                RSSI = [int]$parts[2]
                FirstSeen = [long]$parts[3]
                LastSeen = [long]$parts[4]
                Count = [int]$parts[5]
                Locations = if ($parts[6]) { $parts[6] -split ';' } else { @() }
                Name = ""
                SSID = ""
                Manufacturer = ""
            }
        }
    }
    
    Write-Host "  Loaded $($devices.Count) existing devices`n" -ForegroundColor Gray
}

# Find all data files
$dataFiles = Get-ChildItem -Path $DatasetsPath -File | Where-Object {
    $_.Extension -match '\.(log|txt|json|csv)$'
}

if ($dataFiles.Count -eq 0) {
    Write-Host "No data files found in $DatasetsPath" -ForegroundColor Yellow
    exit 0
}

Write-Host "Found $($dataFiles.Count) data files`n" -ForegroundColor Green

# Process each file
$filesProcessed = 0
$totalProcessed = 0

foreach ($file in $dataFiles) {
    $count = Process-File $file.FullName
    $totalProcessed += $count
    $filesProcessed++
}

# Report unknown formats
if ($unknownFormats.Count -gt 0) {
    Write-Host "`nWarning: Could not parse the following files:" -ForegroundColor Yellow
    $unknownFormats | ForEach-Object {
        Write-Host "  - $(Split-Path $_ -Leaf)" -ForegroundColor Red
    }
}

# Write output database
Write-Host "`nWriting database to: $OutputPath" -ForegroundColor Yellow

# Ensure output directory exists
$outputDir = Split-Path -Parent $OutputPath
if ($outputDir -and -not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

# Write header and data
$output = @()
$output += "# Flock Detection Database - Converted from legacy datasets"
$output += "# Conversion Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$output += "# Format: MAC,Type,RSSI,FirstSeen,LastSeen,Count,Locations"

foreach ($mac in $devices.Keys | Sort-Object) {
    $device = $devices[$mac]
    
    $locationsStr = $device.Locations -join ';'
    
    $line = "{0},{1},{2},{3},{4},{5},{6}" -f `
        $device.MAC,
        $device.Type,
        $device.RSSI,
        $device.FirstSeen,
        $device.LastSeen,
        $device.Count,
        $locationsStr
    
    $output += $line
}

$output | Out-File -FilePath $OutputPath -Encoding UTF8

# Summary
Write-Host "`n=== Conversion Complete ===" -ForegroundColor Green
Write-Host "Files processed: $filesProcessed" -ForegroundColor White
Write-Host "Total entries: $totalProcessed" -ForegroundColor White
Write-Host "Unique devices: $($devices.Count)" -ForegroundColor White
Write-Host "Output: $OutputPath" -ForegroundColor White

# Create index file
$indexPath = Join-Path (Split-Path -Parent $OutputPath) "device_index.idx"
Write-Host "`nCreating index file: $indexPath" -ForegroundColor Yellow

$devices.Keys | Sort-Object | Out-File -FilePath $indexPath -Encoding UTF8
Write-Host "Index created with $($devices.Count) devices" -ForegroundColor Gray

# Generate exports
Write-Host "`nGenerating export files..." -ForegroundColor Yellow

$exportDir = Join-Path (Split-Path -Parent $OutputPath) "exports"
if (-not (Test-Path $exportDir)) {
    New-Item -ItemType Directory -Path $exportDir -Force | Out-Null
}

# Export summary CSV with names
$summaryPath = Join-Path $exportDir "summary.csv"
$summary = @("MAC,Type,Name/SSID,RSSI,FirstSeen,LastSeen,DetectionCount,LocationCount,Manufacturer")

foreach ($mac in $devices.Keys | Sort-Object) {
    $device = $devices[$mac]
    $name = if ($device.SSID) { $device.SSID } elseif ($device.Name) { $device.Name } else { "" }
    
    $summary += "{0},{1},{2},{3},{4},{5},{6},{7},{8}" -f `
        $device.MAC,
        $device.Type,
        $name,
        $device.RSSI,
        (Get-Date -UnixTimeSeconds $device.FirstSeen -Format 'yyyy-MM-dd HH:mm:ss'),
        (Get-Date -UnixTimeSeconds $device.LastSeen -Format 'yyyy-MM-dd HH:mm:ss'),
        $device.Count,
        $device.Locations.Count,
        $device.Manufacturer
}

$summary | Out-File -FilePath $summaryPath -Encoding UTF8
Write-Host "  Summary CSV: $summaryPath" -ForegroundColor Gray

# Export GeoJSON (only devices with locations)
$geojsonPath = Join-Path $exportDir "detections.geojson"
$geojson = @{
    type = "FeatureCollection"
    features = @()
}

$devicesWithLocation = 0

foreach ($mac in $devices.Keys) {
    $device = $devices[$mac]
    
    if ($device.Locations.Count -eq 0) { continue }
    
    foreach ($location in $device.Locations) {
        if ([string]::IsNullOrWhiteSpace($location)) { continue }
        
        $coords = $location -split ','
        if ($coords.Count -eq 2) {
            $lat = [double]$coords[0]
            $lon = [double]$coords[1]
            
            $name = if ($device.SSID) { $device.SSID } elseif ($device.Name) { $device.Name } else { "Unknown" }
            
            $feature = @{
                type = "Feature"
                geometry = @{
                    type = "Point"
                    coordinates = @($lon, $lat)
                }
                properties = @{
                    mac = $device.MAC
                    type = $device.Type
                    name = $name
                    rssi = $device.RSSI
                    detections = $device.Count
                    first_seen = $device.FirstSeen
                    last_seen = $device.LastSeen
                }
            }
            
            $geojson.features += $feature
            $devicesWithLocation++
        }
    }
}

if ($geojson.features.Count -gt 0) {
    $geojson | ConvertTo-Json -Depth 10 | Out-File -FilePath $geojsonPath -Encoding UTF8
    Write-Host "  GeoJSON: $geojsonPath ($devicesWithLocation locations)" -ForegroundColor Gray
}
else {
    Write-Host "  GeoJSON: Skipped (no GPS data)" -ForegroundColor DarkGray
}

# Statistics
Write-Host "`n=== Statistics ===" -ForegroundColor Cyan
Write-Host "Total detections: $totalProcessed" -ForegroundColor White
Write-Host "Unique devices: $($devices.Count)" -ForegroundColor White

if ($devices.Count -gt 0) {
    $avgDetections = [math]::Round(($devices.Values | Measure-Object -Property Count -Average).Average, 2)
    $maxDetections = ($devices.Values | Measure-Object -Property Count -Maximum).Maximum
    $devicesWithGPS = ($devices.Values | Where-Object { $_.Locations.Count -gt 0 }).Count
    
    Write-Host "Average detections per device: $avgDetections" -ForegroundColor White
    Write-Host "Max detections for single device: $maxDetections" -ForegroundColor White
    Write-Host "Devices with GPS data: $devicesWithGPS ($([math]::Round(($devicesWithGPS / $devices.Count) * 100, 1))%)" -ForegroundColor White
    
    # Type breakdown
    $typeGroups = $devices.Values | Group-Object -Property Type
    Write-Host "`nDetection types:" -ForegroundColor Cyan
    foreach ($group in $typeGroups | Sort-Object Count -Descending) {
        Write-Host "  $($group.Name): $($group.Count) devices" -ForegroundColor White
    }
}

Write-Host "`nDone!`n" -ForegroundColor Green