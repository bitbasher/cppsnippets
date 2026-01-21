# Create goodIndex test files across all three tiers

$testRoot = "D:\repositories\cppsnippets\cppsnippets\testFileStructure"

# Define the three tiers
$tiers = @(
    @{
        name = "Installation"
        base = "$testRoot\inst\OpenSCAD"
    },
    @{
        name = "Machine"
        base = "$testRoot\pers\Jeff\AppData\local"
    },
    @{
        name = "User"
        base = "$testRoot\pers\Jeff\Documents"
    }
)

foreach ($tier in $tiers) {
    $tierName = $tier.name
    $tierBase = $tier.base
    
    Write-Host "Creating files in $tierName tier: $tierBase"
    
    # Ensure directories exist
    $dirs = @(
        "$tierBase\templates",
        "$tierBase\examples",
        "$tierBase\examples\indexCat",
        "$tierBase\tests",
        "$tierBase\fonts"
    )
    
    foreach ($dir in $dirs) {
        if (!(Test-Path $dir)) {
            New-Item -ItemType Directory -Path $dir -Force | Out-Null
            Write-Host "  Created directory: $dir"
        }
    }
    
    # Create template file (copy from existing if available, else create dummy)
    $templateSrc = "$tierBase\templates\goodIndex.json"
    if (!(Test-Path $templateSrc)) {
        $existing = Get-ChildItem "$tierBase\templates\*.json" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($existing) {
            Copy-Item $existing.FullName $templateSrc -Force
            Write-Host "  Created: goodIndex.json (copied from $($existing.Name))"
        } else {
            @'
{
  "goodIndex": {
    "prefix": "goodIndex",
    "body": ["// goodIndex template"],
    "description": "Test template for index verification"
  }
}
'@ | Out-File $templateSrc -Encoding UTF8 -Force
            Write-Host "  Created: goodIndex.json (dummy)"
        }
    }
    
    # Create example files
    $exampleScadSrc = "$tierBase\examples\goodIndex.scad"
    if (!(Test-Path $exampleScadSrc)) {
        $existing = Get-ChildItem "$tierBase\examples\*.scad" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($existing) {
            Copy-Item $existing.FullName $exampleScadSrc -Force
            Write-Host "  Created: examples/goodIndex.scad (copied)"
        } else {
            "// goodIndex example" | Out-File $exampleScadSrc -Encoding UTF8 -Force
            Write-Host "  Created: examples/goodIndex.scad (dummy)"
        }
    }
    
    # Create example attachment
    $exampleJpegSrc = "$tierBase\examples\goodIndex.jpeg"
    if (!(Test-Path $exampleJpegSrc)) {
        "fake jpeg attachment" | Out-File $exampleJpegSrc -Encoding UTF8 -Force
        Write-Host "  Created: examples/goodIndex.jpeg"
    }
    
    # Create category folder example
    $catScadSrc = "$tierBase\examples\indexCat\goodIndex.scad"
    if (!(Test-Path $catScadSrc)) {
        $existing = Get-ChildItem "$tierBase\examples\*.scad" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($existing) {
            Copy-Item $existing.FullName $catScadSrc -Force
            Write-Host "  Created: examples/indexCat/goodIndex.scad (copied)"
        } else {
            "// goodIndex category example" | Out-File $catScadSrc -Encoding UTF8 -Force
            Write-Host "  Created: examples/indexCat/goodIndex.scad (dummy)"
        }
    }
    
    # Create category attachment
    $catJpegSrc = "$tierBase\examples\indexCat\goodIndex.jpeg"
    if (!(Test-Path $catJpegSrc)) {
        "fake jpeg attachment category" | Out-File $catJpegSrc -Encoding UTF8 -Force
        Write-Host "  Created: examples/indexCat/goodIndex.jpeg"
    }
    
    # Create tests files
    $testScadSrc = "$tierBase\tests\goodIndex.scad"
    if (!(Test-Path $testScadSrc)) {
        $existing = Get-ChildItem "$tierBase\examples\*.scad" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($existing) {
            Copy-Item $existing.FullName $testScadSrc -Force
            Write-Host "  Created: tests/goodIndex.scad (copied)"
        } else {
            "// goodIndex test" | Out-File $testScadSrc -Encoding UTF8 -Force
            Write-Host "  Created: tests/goodIndex.scad (dummy)"
        }
    }
    
    # Create tests attachment
    $testJpegSrc = "$tierBase\tests\goodIndex.jpeg"
    if (!(Test-Path $testJpegSrc)) {
        "fake jpeg attachment test" | Out-File $testJpegSrc -Encoding UTF8 -Force
        Write-Host "  Created: tests/goodIndex.jpeg"
    }
    
    # Create fonts file
    $fontSrc = "$tierBase\fonts\goodIndex.ttf"
    if (!(Test-Path $fontSrc)) {
        "fake ttf font" | Out-File $fontSrc -Encoding UTF8 -Force
        Write-Host "  Created: fonts/goodIndex.ttf"
    }
    
    Write-Host ""
}

Write-Host "All goodIndex test files created successfully!"
