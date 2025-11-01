# Remote Desktop - Hızlı Başlangıç Kılavuzu

## 🎉 Tebrikler! Derleme Başarılı!

Aşağıdaki dosyalar oluşturuldu:
- ✅ `build/RemoteServer.exe` (2.5 MB)
- ✅ `build/RemoteClient.exe` (2.6 MB)

## 🚀 Hızlı Test (Aynı Bilgisayarda)

### Adım 1: Server'ı Başlat
```cmd
run_server.bat
```
veya
```cmd
.\build\RemoteServer.exe
```

**Çıktı göreceksiniz:**
```
==================================
  Remote Desktop Server
==================================
Server started on port 5900
Screen size: 1920x1080
Waiting for connections...
```

### Adım 2: Client'ı Başlat (Yeni Terminal)
```cmd
run_client.bat
```
veya
```cmd
.\build\RemoteClient.exe 127.0.0.1 5900
```

**Ne olacak:**
- Bir pencere açılacak
- Kendi ekranınızı gerçek zamanlı göreceksiniz!
- 30 FPS hedef hız
- JPEG sıkıştırma ile ~70-80% veri tasarrufu

**🎮 Remote Control Nasıl Kullanılır:**
- **F1** tuşuna basın = Kontrol modunu aç/kapa
- **View Only** mod (varsayılan) = Sadece izle, müdahale etme
- **Controlling** mod (F1'den sonra) = Mouse/keyboard kontrolü aktif
- Pencere başlığı: Mevcut modu gösterir
- Güvenlik için varsayılan VIEW ONLY!

## 🌐 Farklı Bilgisayarlar Arası Kullanım

### Server Bilgisayarda (Ekranı paylaşılacak):
1. IP adresini öğren:
   ```cmd
   ipconfig
   ```
   Örnek: `192.168.1.100`

2. Server'ı başlat:
   ```cmd
   .\build\RemoteServer.exe
   ```

3. **Windows Firewall:** Port 5900'ü aç
   ```powershell
   New-NetFirewallRule -DisplayName "Remote Desktop Server" -Direction Inbound -LocalPort 5900 -Protocol TCP -Action Allow
   ```

### Client Bilgisayarda (Bağlanacak):
```cmd
.\build\RemoteClient.exe 192.168.1.100 5900
```

## 🔧 Sorun Giderme

### "Connection failed" hatası:
- ✅ IP adresini doğrula: `ping 192.168.1.100`
- ✅ Firewall kontrolü: Windows Defender'da port 5900 açık mı?
- ✅ Server çalışıyor mu?

### Port zaten kullanımda:
```cmd
netstat -ano | findstr :5900
taskkill /PID <pid_numarası> /F
```

### Düşük FPS:
- JPEG kalitesini düşür: `src/common/protocol.h` -> `JPEG_QUALITY = 50`
- Yeniden derle: `build.bat`

### Bağlantı kopuyor:
- Network hızı yeterli mi kontrol edin
- WiFi yerine Ethernet kullanın

## 📊 Performans Bilgileri

**Mevcut Ayarlar:**
- FPS: 30
- JPEG Kalite: 75
- Protokol: TCP
- Sıkıştırma: JPEG (GDI+)

**1920x1080 için tahmini bant genişliği:**
- Yüksek hareket: ~15-25 Mbps
- Normal kullanım: ~5-10 Mbps
- Statik ekran: ~1-2 Mbps

## 🎯 Gelecek Özellikler

### Şu an YAPAMAYACAKLAR:
- ❌ Mouse/klavye kontrolü (henüz yok)
- ❌ Ses aktarımı
- ❌ Dosya transferi
- ❌ Şifreleme (güvenlik önemli!)

### Yakında eklenecek:
1. **Mouse/Klavye kontrolü** - En yüksek öncelik
2. **DXGI capture** - 2-3x daha hızlı
3. **H.264 codec** - Daha iyi sıkıştırma
4. **TLS şifreleme** - Güvenlik
5. **Kimlik doğrulama** - Parola koruması

## 📝 Kod Yapısı

```
src/
├── common/
│   ├── network.cpp/h        # TCP socket yönetimi
│   ├── compression.cpp/h    # JPEG encode/decode
│   └── protocol.h           # Paket yapıları
├── server/
│   ├── screen_capture.cpp/h # GDI BitBlt ekran yakalama
│   └── server.cpp/h         # Server ana mantığı
└── client/
    ├── client.cpp/h         # Network client
    └── renderer.cpp/h       # Win32 görüntü çizme
```

## 🛠️ Geliştirme

### Kod değişikliği yaptıktan sonra:
```cmd
build.bat
```

### Debug modunda derleme:
```cmd
g++ -g -DDEBUG ... (tüm dosyalar)
```

### CMake ile (önerilen):
```cmd
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 📞 Destek

### Log dosyaları:
- Server: Console çıktısı
- Client: Console çıktısı + Windows Event Viewer

### Hata raporlama:
1. Konsol çıktısını kaydet
2. Ekran görüntüsü al
3. `ipconfig` ve `netstat -ano` çıktıları

## ⚖️ Lisans

Bu proje **eğitim amaçlıdır**. 
- ✅ Öğrenmek için kullanabilirsiniz
- ✅ Modifiye edebilirsiniz
- ⚠️ Ticari kullanım için uygun lisans ekleyin
- ⚠️ Güvenlik: Prodüksiyonda TLS/şifreleme ekleyin!

## 🎓 Teknik Detaylar

**Kullanılan Teknolojiler:**
- C++17
- WinSock2 (Network)
- GDI/GDI+ (Screen capture & JPEG)
- Win32 API (GUI)

**Mimari:**
- Client-Server model
- Single-threaded server (şimdilik)
- Asenkron network I/O
- Frame-based streaming

**Veri Akışı:**
```
[Screen Capture] → [JPEG Compress] → [TCP Send] → 
→ [TCP Receive] → [JPEG Decompress] → [Display]
```

## 🏆 Başarılar

✅ Windows-to-Windows remote desktop çalışıyor!
✅ Gerçek zamanlı ekran paylaşımı
✅ JPEG sıkıştırma ile verimli bandwidth
✅ Basit ve temiz kod yapısı
✅ Statik linklenmiş executable'lar (bağımlılık yok)

---

**İyi Kullanımlar! 🚀**
