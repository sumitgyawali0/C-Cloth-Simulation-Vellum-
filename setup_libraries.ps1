# Setup Libraries for Cloth Simulation
# This script downloads and sets up GLFW, GLEW, and GLM libraries

Write-Host "Setting up libraries for Cloth Simulation..." -ForegroundColor Green

# Create directories if they don't exist
if (!(Test-Path "external\include")) { New-Item -ItemType Directory -Path "external\include" -Force }
if (!(Test-Path "external\lib")) { New-Item -ItemType Directory -Path "external\lib" -Force }

# Download GLFW
Write-Host "Downloading GLFW..." -ForegroundColor Yellow
$glfwUrl = "https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.bin.WIN64.zip"
$glfwZip = "glfw.zip"
Invoke-WebRequest -Uri $glfwUrl -OutFile $glfwZip

Write-Host "Extracting GLFW..." -ForegroundColor Yellow
Expand-Archive -Path $glfwZip -DestinationPath "external" -Force
Remove-Item $glfwZip

# Copy GLFW files
if (Test-Path "external\glfw-3.3.8.bin.WIN64") {
    Copy-Item "external\glfw-3.3.8.bin.WIN64\include\*" -Destination "external\include" -Recurse -Force
    Copy-Item "external\glfw-3.3.8.bin.WIN64\lib-vc2022\*" -Destination "external\lib" -Force
    Remove-Item "external\glfw-3.3.8.bin.WIN64" -Recurse -Force
}

# Download GLEW
Write-Host "Downloading GLEW..." -ForegroundColor Yellow
$glewUrl = "https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip"
$glewZip = "glew.zip"
Invoke-WebRequest -Uri $glewUrl -OutFile $glewZip

Write-Host "Extracting GLEW..." -ForegroundColor Yellow
Expand-Archive -Path $glewZip -DestinationPath "external" -Force
Remove-Item $glewZip

# Copy GLEW files
if (Test-Path "external\glew-2.1.0") {
    Copy-Item "external\glew-2.1.0\include\*" -Destination "external\include" -Recurse -Force
    Copy-Item "external\glew-2.1.0\lib\Release\x64\*" -Destination "external\lib" -Force
    Remove-Item "external\glew-2.1.0" -Recurse -Force
}

# Download GLM
Write-Host "Downloading GLM..." -ForegroundColor Yellow
$glmUrl = "https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"
$glmZip = "glm.zip"
Invoke-WebRequest -Uri $glmUrl -OutFile $glmZip

Write-Host "Extracting GLM..." -ForegroundColor Yellow
Expand-Archive -Path $glmZip -DestinationPath "external" -Force
Remove-Item $glmZip

# Copy GLM files
if (Test-Path "external\glm") {
    Copy-Item "external\glm\glm" -Destination "external\include" -Recurse -Force
    Remove-Item "external\glm" -Recurse -Force
}

Write-Host "Library setup complete!" -ForegroundColor Green
Write-Host "You can now run: .\build.bat" -ForegroundColor Cyan

