# Windows Code Signing for Inviwo

This document explains Windows code signing and how it is integrated into Inviwo's CI/CD pipeline.

## What is Windows Code Signing?

When distributing Windows applications and installers, users may see warnings like "Windows protected your PC" or "Unknown publisher" if the software isn't digitally signed. Code signing:

1. **Verifies authenticity**: Confirms the software comes from a trusted publisher
2. **Ensures integrity**: Verifies the software hasn't been tampered with since signing
3. **Improves user trust**: Reduces or eliminates security warnings during installation
4. **Enables SmartScreen reputation**: Signed applications build reputation with Microsoft SmartScreen

## How Windows Code Signing Works

### Certificate Types

1. **Standard Code Signing Certificate**: Signs applications and shows your organization name
2. **EV (Extended Validation) Code Signing Certificate**: Provides immediate SmartScreen reputation and is required for kernel-mode drivers

### The Signing Process

1. **Obtain a code signing certificate** from a trusted Certificate Authority (CA):
   - DigiCert
   - Sectigo (formerly Comodo)
   - GlobalSign
   - SSL.com

2. **Sign executables and DLLs** using Microsoft's SignTool or similar tools
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
2. **Certificate Storage**: EV certificates typically require a Hardware Security Module (HSM) or cloud-based signing service
3. **GitHub Secrets**: Store certificate and credentials securely

### GitHub Secrets Required

Add these secrets to your GitHub repository:

| Secret Name | Description |
|-------------|-------------|
| `WINDOWS_SIGNING_CERT` | Base64-encoded PFX certificate (for standard certificates) |
| `WINDOWS_SIGNING_CERT_PASSWORD` | Password for the PFX certificate |
| `WINDOWS_SIGNING_TIMESTAMP_URL` | Timestamp server URL (e.g., `http://timestamp.digicert.com`) |

For EV certificates with cloud signing services (recommended):

| Secret Name | Description |
|-------------|-------------|
| `AZURE_KEY_VAULT_URI` | Azure Key Vault URI |
| `AZURE_CLIENT_ID` | Azure AD application client ID |
| `AZURE_CLIENT_SECRET` | Azure AD application client secret |
| `AZURE_TENANT_ID` | Azure AD tenant ID |
| `AZURE_CERT_NAME` | Certificate name in Key Vault |

### Signing Methods

#### Method 1: SignTool with PFX Certificate (Standard Certificates)

```powershell
# Sign a single file
signtool sign /f certificate.pfx /p $password /t http://timestamp.digicert.com /fd sha256 /v myapp.exe

# Sign multiple files
Get-ChildItem -Path "build/bin" -Include "*.exe","*.dll" -Recurse | ForEach-Object {
    signtool sign /f certificate.pfx /p $password /t http://timestamp.digicert.com /fd sha256 /v $_.FullName
}
```

#### Method 2: Azure Key Vault (EV Certificates - Recommended)

Azure Key Vault provides secure cloud-based signing for EV certificates:

```powershell
# Install Azure SignTool
dotnet tool install --global AzureSignTool

# Sign files
AzureSignTool sign -kvu $AZURE_KEY_VAULT_URI -kvi $AZURE_CLIENT_ID -kvs $AZURE_CLIENT_SECRET -kvt $AZURE_TENANT_ID -kvc $AZURE_CERT_NAME -tr http://timestamp.digicert.com -td sha256 myapp.exe
```

#### Method 3: SSL.com eSigner (Cloud-based EV Signing)

SSL.com offers cloud-based signing without hardware requirements:

```powershell
# Install CodeSignTool
# Download from SSL.com

# Sign files
CodeSignTool sign -username=$USERNAME -password=$PASSWORD -totp_secret=$TOTP_SECRET -credential_id=$CREDENTIAL_ID -input_file_path=myapp.exe
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
   - Standard certificates require reputation building
   - Consider upgrading to an EV certificate
   - Ensure proper timestamping

## Best Practices

1. **Use EV certificates** for immediate SmartScreen trust
2. **Always timestamp signatures** to maintain validity after certificate expiration
3. **Sign all executables and DLLs**, not just the main application
4. **Sign the installer** as the final step
5. **Store certificates securely** using HSM or cloud-based solutions
6. **Rotate credentials regularly** and use short-lived tokens where possible
7. **Test signed installers** on a clean Windows machine

## Cost Considerations

| Certificate Type | Approximate Annual Cost | SmartScreen Impact |
|-----------------|------------------------|-------------------|
| Standard Code Signing | $200-400/year | Builds reputation over time |
| EV Code Signing | $400-600/year | Immediate trust |
| Azure Key Vault | Usage-based | N/A (certificate storage only) |

## References

- [Microsoft Code Signing Documentation](https://docs.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools)
- [SignTool Documentation](https://docs.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [Azure Key Vault Code Signing](https://docs.microsoft.com/en-us/azure/key-vault/)
- [NSIS Code Signing](https://nsis.sourceforge.io/Signing_NSIS_installers)
