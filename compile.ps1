$filesToCompile = "encode", "decode"

Foreach ($fileToCompile in $filesToCompile) {
    $cFile = $fileToCompile + '.c'
    $asmFile = $fileToCompile + '.asm'
    $objFile = $fileToCompile + '.obj'  # Object code that is legible from Windows
    $lstFile = $fileToCompile + '.lst'
    $oFile = $fileToCompile + '.O'      # Object code that is legible from Classic99

    write-host 'Compling and assmbling' $cFile ' to ' $oFile

    & 'C:\Program Files Uninstalled\Ti994w\Utl\C99C.exe' $cFile $asmFile
    & 'C:\Program Files Uninstalled\Ti994w\Utl\XA99.EXE' -i $asmFile -o $objFile -l $lstFile -v -r -S
    
    # Add TIFILES header
    if (Test-Path $oFile) {
        Remove-Item $oFile
    }
    $recordCount = 0
    foreach($line in Get-Content $objFile) {
        $recordCount = $recordCount + 1
    }
    $sectorCount = [math]::Ceiling($recordCount / 3.0)
    if ($sectorCount -gt 255) {
        throw 'the sector count is larger than 255. fix the script.'
    }
    $bytes = [System.Collections.ArrayList](
       0x07, 0x54, 0x49, 0x46, 0x49, 0x4C, 0x45, 0x53, 0x00, $sectorCount, 0x00, 0x03, 0xA0, 0x50, 0x25, 0x01,
       0x44, 0x45, 0x43, 0x4F, 0x44, 0x45, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x82, 0x6F,
       0x2B, 0x39, 0x82, 0x6F, 0x2B, 0x39, 0xFF, 0xFF, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
       0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
       0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
       0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
       0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
       0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
    )
    # Copy the file contents, but remove line-endings
    $lineInSector = 0
    foreach($line in Get-Content $objFile) {
        $lineAsBytes = [System.Text.Encoding]::ASCII.GetBytes($line);
        $bytes.AddRange($lineAsBytes)
        $lineInSector = $lineInSector + 1
        if($lineInSector -eq 3){
            $lineInSector = 0
            $bytes.AddRange((
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
            ))
        }
    }
    [IO.File]::WriteAllBytes($oFile, $bytes)
 
    # Comment any of these lines out if you wish to read the auto-produced files.
    Remove-Item $asmFile
    Remove-Item $objFile
    Remove-Item $lstFile
}