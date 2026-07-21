# Deployment Guide

## macOS

### Create DMG (Disk Image)

```bash
./scripts/create_dmg.sh
```

### Code Signing

```bash
codesign -s - ./build/bin/CarlosOkupa.app
```

### Notarization (Apple Developer Certificate Required)

```bash
# Submit for notarization
xcrun altool --notarize-app -f CarlosOkupa.dmg -t osx -u <apple_id> -p <app_password>

# Check status
xcrun altool --notarization-history <request_uuid> -u <apple_id> -p <app_password>
```

## Windows

### Create Installer (NSIS)

```bash
./scripts/create_installer.sh
```

## Distribution

- **macOS**: CarlosOkupa.dmg
- **Windows**: CarlosOkupa-Setup.exe

⚠️ **Current Status**: Development builds only. Not signed or notarized.
