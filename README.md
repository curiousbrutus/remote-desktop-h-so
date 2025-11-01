# Remote Desktop - Kullanım Kılavuzu

## Gereksinimler

- **Windows 10/11**
- **CMake 3.15+**
- **Visual Studio 2019/2022** (veya MinGW-w64)
- **C++17 desteği**

## Hızlı Başlangıç

### Seçenek 1: Hazır .exe kullan (Önerilen - Hızlı)

Repository'de zaten derlenmiş `.exe` dosyaları var:
- `build/RemoteServer.exe` - Sunucu uygulaması
- `build/RemoteClient.exe` - İstemci uygulaması

**Doğrudan kullanabilirsiniz!** Derlemeye gerek yok.

```powershell
# Server başlat
.\build\RemoteServer.exe

# Client başlat (başka terminal/bilgisayarda)
.\build\RemoteClient.exe 127.0.0.1 5900
```

Veya tek tıkla:
```powershell
.\run_server.bat
.\run_client.bat
```

### Seçenek 2: Kendin Derle

#### Gereksinimler Kurulumu

**Otomatik kurulum (önerilen):**
```powershell
.\setup.ps1
```

**Manuel kurulum:**
- MinGW-w64: https://winlibs.com/
- CMake (opsiyonel): https://cmake.org/

#### Derleme

**Build script ile (en kolay):**
```powershell
.\build.bat
```

**CMake ile:**

##### Visual Studio ile:

```powershell
cd remote
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

##### MinGW ile:

```powershell
cd remote
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Derleme sonrası:**
- `build/RemoteServer.exe` oluşturulur
- `build/RemoteClient.exe` oluşturulur

## Çalıştırma

### 1. Server'ı Başlat

Ekranını paylaşacak bilgisayarda:

```powershell
cd "c:\Users\saglikturizmi-37635\Desktop\Yeni klasör\remote\build"
.\Release\RemoteServer.exe
# veya custom port:
.\Release\RemoteServer.exe 5900
```

### 2. Client'ı Başlat

Bağlanacak bilgisayarda:

```powershell
cd "c:\Users\saglikturizmi-37635\Desktop\Yeni klasör\remote\build"
.\Release\RemoteClient.exe 192.168.1.100 5900
# veya localhost için:
.\Release\RemoteClient.exe 127.0.0.1 5900
```

## Özellikler

### Mevcut:
- ✅ Windows ekran yakalama (GDI BitBlt)
- ✅ JPEG sıkıştırma (GDI+)
- ✅ TCP/IP üzerinden stream
- ✅ 30 FPS hedef frame rate
- ✅ Client GUI (Win32)
- ✅ Çoklu monitör desteği (primary monitor)

### Gelecek Geliştirmeler:
- ⏳ Mouse/Keyboard kontrolü
- ⏳ DXGI Desktop Duplication (daha hızlı)
- ⏳ H.264 video codec
- ⏳ TLS/SSL şifreleme
- ⏳ Kimlik doğrulama
- ⏳ Multi-monitor seçimi
- ⏳ Clipboard paylaşımı

## Sorun Giderme

**Port zaten kullanımda:**
```powershell
netstat -ano | findstr :5900
taskkill /PID <pid> /F
```

**Firewall engeli:**
Windows Defender Firewall'da port 5900'ü açın.

**Bağlantı kurulamıyor:**
- IP adresini doğrulayın: `ipconfig`
- Ping test edin: `ping <server_ip>`
- Firewall kurallarını kontrol edin

## Mimari

```
Server: Screen Capture → JPEG Compress → TCP Send (30 FPS)
Client: TCP Receive → JPEG Decompress → Render → Display
```

## Lisans

Bu proje eğitim amaçlıdır. Ticari kullanım için uygun lisans ekleyin.
