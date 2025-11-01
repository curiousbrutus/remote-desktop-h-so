# Remote Desktop - Windows Screen Sharing Application

[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-orange.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-Educational-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](build/)

**Tam özellikli Windows remote desktop uygulaması** - Gerçek zamanlı ekran paylaşımı, JPEG sıkıştırma, TCP/IP streaming ile C++ ile yazılmış native uygulama.

## ⚡ Hızlı Başlat

**Derlemeye gerek yok!** Hazır `.exe` dosyaları dahil:

```powershell
# 1. Repository'yi klonla
git clone https://github.com/curiousbrutus/remote-desktop-h-so.git
cd remote-desktop-h-so

# 2. Server'ı başlat
.\run_server.bat

# 3. Client'ı başlat (yeni terminal)
.\run_client.bat
```

**🎬 Hazır! Ekranınızı gerçek zamanlı görüyorsunuz!**

## 📥 İndir ve Kullan

Sadece `.exe` dosyalarını indirmek isterseniz:
1. [build/RemoteServer.exe](build/RemoteServer.exe) - Server uygulaması (2.6 MB)
2. [build/RemoteClient.exe](build/RemoteClient.exe) - Client uygulaması (2.7 MB)

Çalıştırın ve kullanın! Bağımlılık yok.

## ✨ Özellikler

- ✅ **Gerçek zamanlı ekran paylaşımı** - 30 FPS
- ✅ **JPEG sıkıştırma** - %70-80 veri tasarrufu
- ✅ **TCP/IP streaming** - Güvenilir veri aktarımı
- ✅ **Win32 GUI** - Native Windows uygulaması
- ✅ **Otomatik ölçeklendirme** - Pencere boyutuna uyum
- ✅ **Çoklu monitör desteği** - Primary monitor
- ✅ **Bağımlılık yok** - Statik linklenmiş

## 📋 Gereksinimler

### Kullanım İçin (Hazır .exe)
- **Windows 10/11** - 64-bit
- **Network bağlantısı** - LAN veya Internet

### Derleme İçin (Opsiyonel)
- **Windows 10/11**
- **MinGW-w64** veya **Visual Studio 2019/2022**
- **CMake 3.15+** (opsiyonel)
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

## 🏗️ Mimari

```
┌─────────────────────────────────────────┐
│              SERVER (Host)              │
│  Screen Capture → JPEG → TCP Send      │
└─────────────────────────────────────────┘
                   ↓ Network
┌─────────────────────────────────────────┐
│            CLIENT (Viewer)              │
│  TCP Receive → JPEG Decode → Display   │
└─────────────────────────────────────────┘
```

**Teknolojiler:**
- C++17 Standard
- WinSock2 (Network)
- GDI/GDI+ (Graphics & JPEG)
- Win32 API (GUI)

## 🚀 Yol Haritası

### Mevcut Sürüm (v1.0)
- ✅ Screen capture (GDI BitBlt)
- ✅ JPEG compression
- ✅ TCP streaming
- ✅ Client GUI

### Gelecek Sürümler
- ⏳ **v1.1:** Mouse/Keyboard remote control
- ⏳ **v1.2:** DXGI Desktop Duplication (2-3x faster)
- ⏳ **v2.0:** H.264 video codec
- ⏳ **v2.1:** TLS/SSL encryption
- ⏳ **v3.0:** Linux support

## 🤝 Katkıda Bulunma

Katkılarınızı bekliyoruz! 

1. Fork yapın
2. Feature branch oluşturun (`git checkout -b feature/amazing-feature`)
3. Commit yapın (`git commit -m 'Add amazing feature'`)
4. Push edin (`git push origin feature/amazing-feature`)
5. Pull Request açın

**Önerilen geliştirmeler:**
- Mouse/Keyboard control implementasyonu
- DXGI capture support
- Performance optimizations
- Bug fixes & testing

## 📄 Lisans

Bu proje **eğitim amaçlıdır**. 
- ✅ Öğrenmek ve geliştirmek için özgürce kullanabilirsiniz
- ⚠️ Prodüksiyon kullanımı için TLS/şifreleme eklemeniz önerilir
- ⚠️ Ticari kullanım için uygun lisans ekleyin

## 📞 İletişim

**Proje Sahibi:** [@curiousbrutus](https://github.com/curiousbrutus)

**Sorular veya öneriler için:**
- GitHub Issues açabilirsiniz
- Pull Request gönderebilirsiniz

## 🙏 Teşekkürler

Bu projeyi kullandığınız için teşekkürler! ⭐ vermeyi unutmayın!

---

**Daha fazla bilgi için:**
- [QUICKSTART.md](QUICKSTART.md) - Detaylı kullanım kılavuzu
- [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) - Proje özeti ve teknik detaylar
