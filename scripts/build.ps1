Write-Output "Checking for build directory"

if (Test-Path build) {
    Write-Output "Build directory exists... removing"
    Remove-Item -Path build -Recurse -Force
}
else {
    Write-Output "Not created yet"
}

Write-Output "Creating build directory"
New-Item -Path .\build -ItemType Directory
Set-Location -Path .\build

Write-Output "Configuring"
cmake -A Win32 ..

if (!($?)) {
    Set-Location -Path ..
    Exit
}

Write-Output "Building"
cmake --build . --config Debug
if (!($?)) {
    Set-Location ..
    Exit
}

Write-Output "Installing"
cmake --install . --config Debug
if (!($?)) {
    Set-Location ..
    Exit
}