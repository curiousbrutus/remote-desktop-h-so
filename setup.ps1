# Remote Desktop - Kurulum ve Derleme Scripti
# PowerShell 5.1+ gerektirir

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Remote Desktop - Setup Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for existing compilers
$hasCompiler = $false

# Check Visual Studio
$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -property installationPath 2>$null

if ($vsPath) {
    Write-Host "[OK] Visual Studio found at: $vsPath" -ForegroundColor Green
    $hasCompiler = $true
}

# Check MinGW
$mingwPath = where.exe g++ 2>$null
if ($mingwPath) {
    Write-Host "[OK] MinGW found at: $mingwPath" -ForegroundColor Green
    $hasCompiler = $true
}

# Check CMake
$cmakePath = where.exe cmake 2>$null
if ($cmakePath) {
    Write-Host "[OK] CMake found at: $cmakePath" -ForegroundColor Green
} else {
    Write-Host "[WARN] CMake not found" -ForegroundColor Yellow
}

Write-Host ""

if (-not $hasCompiler) {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "  NO COMPILER FOUND!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Lütfen aşağıdakilerden birini yükleyin:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "1. Visual Studio 2022 Community (Önerilen):" -ForegroundColor White
    Write-Host "   https://visualstudio.microsoft.com/downloads/" -ForegroundColor Cyan
    Write-Host "   - 'Desktop development with C++' workload'u seçin" -ForegroundColor Gray
    Write-Host ""
    Write-Host "2. MinGW-w64 (Hafif, hızlı):" -ForegroundColor White
    Write-Host "   https://www.mingw-w64.org/downloads/" -ForegroundColor Cyan
    Write-Host "   veya WinLibs: https://winlibs.com/" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "3. Scoop ile otomatik kurulum:" -ForegroundColor White
    Write-Host "   Scoop kurulumu:" -ForegroundColor Gray
    Write-Host "   irm get.scoop.sh | iex" -ForegroundColor Cyan
    Write-Host "   Sonra MinGW kurulumu:" -ForegroundColor Gray
    Write-Host "   scoop install mingw" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "4. Chocolatey ile otomatik kurulum:" -ForegroundColor White
    Write-Host "   choco install mingw -y" -ForegroundColor Cyan
    Write-Host ""
    
    $response = Read-Host "Scoop ile otomatik MinGW kurmak ister misiniz? (E/H)"
    
    if ($response -eq "E" -or $response -eq "e") {
        Write-Host ""
        Write-Host "Scoop kuruluyor..." -ForegroundColor Yellow
        
        try {
            # Install Scoop
            irm get.scoop.sh -outfile 'install_scoop.ps1'
            .\install_scoop.ps1 -RunAsAdmin
            Remove-Item 'install_scoop.ps1'
            
            # Refresh PATH
            $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
            
            Write-Host "MinGW kuruluyor..." -ForegroundColor Yellow
            scoop install mingw cmake
            
            Write-Host ""
            Write-Host "[OK] Kurulum tamamlandı!" -ForegroundColor Green
            Write-Host "Lütfen bu PowerShell penceresini kapatıp yeni bir tane açın." -ForegroundColor Yellow
            Write-Host "Sonra 'build.bat' dosyasını çalıştırın." -ForegroundColor Yellow
            
        } catch {
            Write-Host ""
            Write-Host "[ERROR] Otomatik kurulum başarısız!" -ForegroundColor Red
            Write-Host "Lütfen manuel olarak derleyici kurun." -ForegroundColor Yellow
        }
    }
    
    exit 1
}

Write-Host "========================================" -ForegroundColor Green
Write-Host "  Derleme araçları hazır!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Projeyi derlemek için:" -ForegroundColor White
Write-Host "  .\build.bat" -ForegroundColor Cyan
Write-Host ""
Write-Host "veya CMake ile:" -ForegroundColor White
if ($cmakePath) {
    Write-Host "  cd build" -ForegroundColor Cyan
    Write-Host "  cmake .. -G 'MinGW Makefiles'" -ForegroundColor Cyan
    Write-Host "  cmake --build . --config Release" -ForegroundColor Cyan
} else {
    Write-Host "  (CMake kurulu değil)" -ForegroundColor Yellow
}
Write-Host ""
