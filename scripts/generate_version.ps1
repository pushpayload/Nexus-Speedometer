# Get current date and time
$Year = (Get-Date).Year
$Month = (Get-Date).Month
$Day = (Get-Date).Day
$Hour = (Get-Date).Hour
$Minute = (Get-Date).Minute

# Calculate revision (hours * 60 + minutes)
$Revision = ($Hour * 60 + $Minute)

# Create Version.h content
$Content = @"
#pragma once
#define V_MAJOR $Year
#define V_MINOR $Month
#define V_BUILD $Day
#define V_REVISION $Revision
"@

# Write to file, creating the directory if it doesn't exist
$OutputPath = "$PSScriptRoot/../src/version.h"
$OutputDir = Split-Path $OutputPath -Parent
if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

# Write the content to the file
$Content | Set-Content -Path $OutputPath -Force
