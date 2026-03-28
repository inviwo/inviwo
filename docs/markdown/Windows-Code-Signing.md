# Windows Code Signing for Inviwo

This document explains Windows code signing and how it is integrated into Inviwo's CI/CD pipeline.

## What is Windows Code Signing?

When distributing Windows applications and installers, users may see warnings like "Windows protected your PC" or "Unknown publisher" if the software isn't digitally signed. Code signing:

1. **Verifies authenticity**: Confirms the software comes from a trusted publisher
2. **Ensures integrity**: Verifies the software hasn't been tampered with since signing
3. **Improves user trust**: Reduces or eliminates security warnings during installation
4. **Enables SmartScreen reputation**: Signed applications build reputation with Microsoft SmartScreen

## How Windows Code Signing Works

### The Signing Process

1. **Obtain a code signing certificate** from a trusted Certificate Authority (CA):
   - DigiCert
   - Sectigo (formerly Comodo)
   - GlobalSign
   - SSL.com

2. **Sign executables and DLLs** using Microsoft's SignTool
3. **Timestamp the signature** to ensure validity even after certificate expiration
4. **Sign the installer** (NSIS, MSI, etc.) for an additional layer of trust

### What Needs to be Signed

For Inviwo, the following files should be signed:
- `inviwo.exe` - The main application
- `inviwo-cli.exe` - The CLI application  
- All `.dll` files produced by the build
- The NSIS installer `.exe` file

## CI Integration for Code Signing

### Prerequisites

1. **Code Signing Certificate**: Purchase from a trusted CA
2. **GitHub Secrets**: Store certificate and credentials securely

### GitHub Secrets Required

Add these secrets to your GitHub repository:

| Secret Name | Description |
|-------------|-------------|
| `WINDOWS_SIGNING_CERT` | Base64-encoded PFX certificate |
| `WINDOWS_SIGNING_CERT_PASSWORD` | Password for the PFX certificate |

### Signing with SignTool

```powershell
# Sign a single file
signtool sign /f certificate.pfx /p $password /tr http://timestamp.digicert.com /fd sha256 /v myapp.exe

# Sign multiple files
Get-ChildItem -Path "build/bin" -Include "*.exe","*.dll" -Recurse | ForEach-Object {
    signtool sign /f certificate.pfx /p $password /tr http://timestamp.digicert.com /fd sha256 /v $_.FullName
}
```

## Workflow Integration

The signing process is integrated into the GitHub Actions workflow:

1. **Build Stage**: Compile all executables and DLLs
2. **Sign Executables**: Sign all `.exe` and `.dll` files before packaging
3. **Create Installer**: Run CPack to create the NSIS installer
4. **Sign Installer**: Sign the final `.exe` installer
5. **Upload Artifact**: Upload the signed installer

### Environment Variables

The following CMake options control signing behavior:

- `IVW_SIGN_WINDOWS_BINARIES`: Enable/disable Windows code signing (default: OFF)
- `IVW_WINDOWS_SIGN_IDENTITY`: Certificate identity or thumbprint
- `IVW_WINDOWS_TIMESTAMP_URL`: Timestamp server URL

## Verification

After signing, verify the signature:

```powershell
# Verify signature
signtool verify /pa /v myapp.exe

# Check signature details in PowerShell
Get-AuthenticodeSignature myapp.exe
```

## Troubleshooting

### Common Issues

1. **"The specified timestamp server could not be reached"**
   - Try a different timestamp URL
   - Check network connectivity
   - Common timestamp URLs:
     - `http://timestamp.digicert.com`
     - `http://timestamp.sectigo.com`
     - `http://timestamp.globalsign.com/tsa/r6advanced1`

2. **"The file is being used by another process"**
   - Ensure all build processes have completed
   - Close any applications using the files

3. **"SignTool Error: No certificates were found"**
   - Verify the certificate is properly installed
   - Check the certificate thumbprint/identity

4. **SmartScreen warnings still appear**
   - Certificates require reputation building over time
   - Ensure proper timestamping

## Best Practices

1. **Always timestamp signatures** to maintain validity after certificate expiration
2. **Sign all executables and DLLs**, not just the main application
3. **Sign the installer** as the final step
4. **Store certificates securely**
5. **Rotate credentials regularly** and use short-lived tokens where possible
6. **Test signed installers** on a clean Windows machine

## References

- [Microsoft Code Signing Documentation](https://docs.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools)
- [SignTool Documentation](https://docs.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [NSIS Code Signing](https://nsis.sourceforge.io/Signing_NSIS_installers)
