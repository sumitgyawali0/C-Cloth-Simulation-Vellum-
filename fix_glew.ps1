# Fix GLEW - Download the correct package with DLL files
Write-Host "Fixing GLEW package..." -ForegroundColor Green

# Download GLEW with DLL files
Write-Host "Downloading GLEW with DLL files..." -ForegroundColor Yellow
$glewUrl = "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip"
$glewZip = "glew_fix.zip"
Invoke-WebRequest -Uri $glewUrl -OutFile $glewZip

Write-Host "Extracting GLEW..." -ForegroundColor Yellow
Expand-Archive -Path $glewZip -DestinationPath "external" -Force
Remove-Item $glewZip

# Copy GLEW DLL files
if (Test-Path "external\glew-2.1.0") {
    Write-Host "Copying GLEW DLL files..." -ForegroundColor Yellow
    Copy-Item "external\glew-2.1.0\bin\Release\x64\*" -Destination "external\lib" -Force
    Remove-Item "external\glew-2.1.0" -Recurse -Force
    Write-Host "GLEW DLL files copied successfully!" -ForegroundColor Green
} else {
    Write-Host "Failed to find GLEW package" -ForegroundColor Red
}

Write-Host "GLEW fix complete!" -ForegroundColor Green
Write-Host "You can now rebuild and run the project" -ForegroundColor Cyan

