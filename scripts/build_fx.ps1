[CmdletBinding()]
param(
    [ValidateSet('win64', 'win32')]
    [string]$Arch = 'win64',

    [AllowNull()]
    [object]$Configuration = 'all',

    [int]$Parallel = [Math]::Max(1, [Environment]::ProcessorCount),

    [int]$BuildTimeoutSeconds = 120,

    [string]$BuildDir,

    [string]$ThreadXDir,

    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'fx_windows_common.ps1')

$repoRoot = Split-Path -Parent $PSScriptRoot
$settings = Get-PortSettings -SelectedArch $Arch

if (-not $BuildDir) {
    $BuildDir = Join-Path $repoRoot "build\tests\$Arch"
}

if (-not $ThreadXDir) {
    $ThreadXDir = Join-Path (Split-Path -Parent $repoRoot) 'threadx-fd-codex'
}

$selectedConfigurations = Resolve-RegressionConfigurations -RequestedConfigurations $Configuration
Write-Host "Selected configurations: $($selectedConfigurations -join ', ')"

Enter-VisualStudioDevShell -VsArch $settings.VsArch

foreach ($currentConfiguration in $selectedConfigurations) {
    $currentBuildDirName = Get-RegressionBuildDirectoryName -ConfigurationName $currentConfiguration
    $currentBuildDir = Join-Path $BuildDir $currentBuildDirName

    if ($Clean) {
        Remove-BuildDirectory -Path $currentBuildDir -RepoRoot $repoRoot
    }

    Remove-NinjaLock -Path $currentBuildDir

    Write-Host "Configuring $Arch / $currentConfiguration"
    Invoke-NativeCommand -FilePath 'cmake' -Arguments @(
        '-S', (Join-Path $repoRoot 'test\cmake'),
        '-B', $currentBuildDir,
        '-G', 'Ninja',
        '-DCMAKE_C_COMPILER_FORCED=TRUE',
        '-DCMAKE_C_COMPILER_WORKS=TRUE',
        '-DCMAKE_C_ABI_COMPILED=TRUE',
        # FileX's root CMakeLists.txt declares LANGUAGES C ASM; suppress the
        # ASM compiler probe because the win64 port has no assembly sources.
        '-DCMAKE_ASM_COMPILER_FORCED=TRUE',
        '-DCMAKE_ASM_COMPILER_WORKS=TRUE',
        "-DCMAKE_BUILD_TYPE=$currentConfiguration",
        "-DTHREADX_ARCH=$($settings.FileXArch)",
        "-DTHREADX_TOOLCHAIN=$($settings.FileXToolchain)",
        "-DTHREADX_SOURCE_DIR=$ThreadXDir"
    )

    Write-Host "Building $Arch / $currentConfiguration"
    Invoke-CMakeBuild -BuildDir $currentBuildDir -Parallel $Parallel -TimeoutSeconds $BuildTimeoutSeconds
}
