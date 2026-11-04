<#
Default: create/review a plan only. -Prepare explicitly allows header
configuration (including compiler probes) and import-library generation.
Run in an x64 VS 2022 developer shell when using -Prepare.
#>
[CmdletBinding()]
param
(
    [string]$VtkSource = (Join-Path $PSScriptRoot "../../../VTK-9.4.2"),
    [Parameter(Mandatory = $true)][string]$InspectionReport,
    [Parameter(Mandatory = $true)][string]$SettingsFile,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [string]$Python = "python",
    [string]$CMake = "cmake",
    [string]$Librarian = "lib.exe",
    [switch]$Prepare
)

$ErrorActionPreference = "Stop"
$sdkArguments = @(
    "-B",
    (Join-Path $PSScriptRoot "prepare_cubit_vtk_sdk.py"),
    "--source", $VtkSource,
    "--report", $InspectionReport,
    "--settings", $SettingsFile,
    "--output", $OutputDirectory,
    "--cmake", $CMake,
    "--lib-exe", $Librarian
)
if ($Prepare)
{
    $sdkArguments += "--execute"
}
& $Python @sdkArguments
if ($LASTEXITCODE -ne 0)
{
    throw "VTK SDK preparation failed with exit code $LASTEXITCODE."
}
