
# Function to generate a random string
function Get-RandomString($length) {
    $chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    $randomString = -join ((65..90) + (97..122) + (48..57) | Get-Random -Count $length | ForEach-Object {[char]$_})
    return $randomString
}

# Generate unique names for the certificate
$randomName = (Get-RandomString -length 4) + ".projects.google.com"
# $issuerName = (Get-RandomString -length 8) + ".projects.google.com"

# Create a self-signed code signing certificate
$cert = New-SelfSignedCertificate `
    -Type CodeSigning `
    -Subject "CN=$randomName" `
    -KeyUsage DigitalSignature `
    -FriendlyName $randomName `
    -DnsName $randomName `
    -CertStoreLocation "Cert:\CurrentUser\My"

# Output the certificate details
$cert | Format-List

# Get the thumbprint of the created certificate
$thumbprint = $cert.Thumbprint

# Path to the file you want to sign
# param (
#     [string]$fileToSign
#  )

$fileToSign=$args[0]

# Full path to signtool.exe (adjust the path according to your environment)
#$signtoolPath = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
$signtoolPath = $args[1]


# Check if signtool exists at the specified path
if (-Not (Test-Path $signtoolPath)) {
    Write-Error "signtool.exe not found at the specified path: $signtoolPath"
    exit 1
}

# Sign the file using signtool and the certificate thumbprint
try {
    Start-Process -FilePath $signtoolPath -ArgumentList "sign /fd SHA256 /sha1 $thumbprint /tr http://timestamp.sectigo.com/ /td SHA256 $fileToSign" -Wait -NoNewWindow
} catch {
    Write-Error "Failed to start signtool.exe: $_"
    exit 1
}

# Verify the signing
try {
    Start-Process -FilePath $signtoolPath -ArgumentList "verify /pa $fileToSign" -Wait -NoNewWindow
} catch {
    Write-Error "Failed to start signtool.exe for verification: $_"
    exit 1
}

Write-Output "File signed with certificate: CN=$randomName"
