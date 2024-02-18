

if ( (Get-OdbcDsn -Name "jde_app" -ErrorAction Ignore) -ne $null){
  Remove-OdbcDsn -Name "jde_app" -Platform "64-bit" -DsnType "System"
}
if ( (Get-OdbcDsn -Name "jde_iot" -ErrorAction Ignore) -ne $null){
  Remove-OdbcDsn -Name "jde_iot" -Platform "64-bit" -DsnType "System"
}
[Environment]::SetEnvironmentVariable('Jde_App_Connection', $null, 'Machine')
[Environment]::SetEnvironmentVariable('Jde_Iot_Connection', $null, 'Machine')
if( Test-Path -Path $Env:PROGRAMFILES/jde-cpp ) {
  Remove-Item -LiteralPath $Env:PROGRAMFILES/jde-cpp -Force -Recurse
}
if( Test-Path -Path $Env:ProgramData/jde-cpp ) {
  Remove-Item -LiteralPath $Env:ProgramData/jde-cpp -Force -Recurse
}
if( Test-Path -Path "$([Environment]::GetFolderPath("CommonDesktopDirectory"))/JdeIotWeb.bat" ) {
  Remove-Item -LiteralPath "$([Environment]::GetFolderPath("CommonDesktopDirectory"))/JdeIotWeb.bat"
}
if( Test-Path -Path "$([Environment]::GetFolderPath("CommonDesktopDirectory"))/JdeDBSetup.ps1" ) {
  Remove-Item -LiteralPath "$([Environment]::GetFolderPath("CommonDesktopDirectory"))/JdeDBSetup.ps1"
}


$Server = "localhost"
$AppDB = "jde_app"
$IotDB = "jde_iot"
#Invoke-Sqlcmd -Query "if exists(select * from sys.databases where name = '$($AppDB)') drop database $($AppDB)" -ServerInstance "$($Server)"
#Invoke-Sqlcmd -Query "if exists(select * from sys.databases where name = '$($IotDB)') drop database $($IotDB)" -ServerInstance "$($Server)"
Read-Host -Prompt "Press Enter to exit"
