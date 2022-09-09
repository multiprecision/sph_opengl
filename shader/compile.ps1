$path = [System.IO.Path]::GetFullPath((Join-Path $pwd "../bin"))
If(!(test-path -PathType container $path))
{
      New-Item -ItemType Directory -Path $path
}
Get-ChildItem -Recurse -Include ("*.vert", "*.frag", "*.comp", "*.geom", "*.tesc", "*.tese") | Foreach {
  $outfile = [System.IO.Path]::GetFullPath((Join-Path (Join-Path $pwd "../bin") ($_.Name + ".spv")))
  & $env:VULKAN_SDK\Bin\glslangvalidator.exe -V $_.FullName -o $outfile
}
