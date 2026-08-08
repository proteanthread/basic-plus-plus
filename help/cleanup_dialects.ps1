$targetDir = 'c:\Users\rtdos\GitHub\basic-plus-plus\help'
$filesToDelete = @(
    'Creating_Dialects.txt',
    'Dialect_Architecture.txt',
    'Dialect_Audit_Overview.txt',
    'Mixing_Dialects.txt',
    'Specific_Machine_Dialects.txt'
)

foreach ($file in $filesToDelete) {
    $path = Join-Path $targetDir $file
    if (Test-Path $path) {
        Remove-Item $path -Force
        Write-Output "Deleted: $path"
    }
}

$files = Get-ChildItem -Path $targetDir -Recurse -Include '*.txt','*.TXT'
foreach ($f in $files) {
    $content = Get-Content $f.FullName -Raw
    $original = $content
    # Delete entire lines that are DIALECT command entries
    $content = $content -replace '(?m)^.*DIALECT_LOAD.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*DIALECT_REGISTER.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*DIALECT_VALIDATE.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*DIALECT_DOC\$.*$\r?\n?', ''
    # Delete lines referencing deleted files
    $content = $content -replace '(?m)^.*Creating_Dialects.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*Dialect_Architecture.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*Dialect_Audit_Overview.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*Mixing_Dialects.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*Specific_Machine_Dialects.*$\r?\n?', ''
    # Delete lines with DIALECT LOAD command examples
    $content = $content -replace '(?m)^.*DIALECT LOAD.*$\r?\n?', ''
    # Delete lines referencing dialect source files
    $content = $content -replace '(?m)^.*dialect_gwbs\.c.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*dialect_qbasic\.c.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*dialect_ecma116\.c.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*source/dialect/.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*BppDialect.*$\r?\n?', ''
    $content = $content -replace '(?m)^.*BASIC_USE_CUSTOM_STATIC_DIALECT.*$\r?\n?', ''
    # Replace phrases (most specific first)
    $content = $content -replace 'BASIC\+\+ dialect ecosystem', 'BASIC++ language system'
    $content = $content -replace 'dialect ecosystem', 'language system'
    $content = $content -replace 'dialect-dependent', 'implementation-dependent'
    $content = $content -replace 'dialect-configurable', 'configurable'
    $content = $content -replace 'dialect-specific', 'implementation-specific'
    $content = $content -replace 'dialect compliance', 'standards compliance'
    $content = $content -replace 'dialect morphing', 'language customization'
    $content = $content -replace 'dialect engine', 'core engine'
    $content = $content -replace 'dialect configuration', 'configuration'
    $content = $content -replace 'dialect config', 'configuration'
    $content = $content -replace 'dialect strictness', 'strictness'
    $content = $content -replace 'dialect constraints', 'compatibility constraints'
    $content = $content -replace 'dialect switching', 'mode switching'
    $content = $content -replace 'dialect toggles', 'configuration toggles'
    $content = $content -replace 'dialect settings', 'configuration settings'
    $content = $content -replace 'cross-dialect', 'cross-platform'
    $content = $content -replace 'multi-dialect', 'multi-platform'
    $content = $content -replace 'internal dialect', 'internal configuration'
    # Keep 'BASIC dialects' as-is (refers to generic historical dialects)
    if ($content -ne $original) {
        Set-Content $f.FullName $content -NoNewline
        Write-Output "Updated: $($f.FullName)"
    }
}
