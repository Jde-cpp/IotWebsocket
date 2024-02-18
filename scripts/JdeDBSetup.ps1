$Server = "localhost"
$AppDB = "jde_app"
$IotDB = "jde_iot"

Invoke-Sqlcmd -Query "if not exists(select * from sys.databases where name = '$($AppDB)') create database $($AppDB)" -ServerInstance "$($Server)"
Invoke-Sqlcmd -Query "if not exists(select * from sys.databases where name = '$($IotDB)') create database $($IotDB)" -ServerInstance "$($Server)"

if ( (Get-OdbcDsn -Name "jde_app" -ErrorAction Ignore) -eq $null){
  Add-OdbcDsn -Name "jde_app" -DriverName "ODBC Driver 17 for SQL Server" -Platform "64-bit" -DsnType "System" -SetPropertyValue @("SERVER=$($SERVER)", "Trusted_Connection=Yes", "DATABASE=$($AppDB)")
}
if ( (Get-OdbcDsn -Name "jde_iot" -ErrorAction Ignore) -eq $null){
  Add-OdbcDsn -Name "jde_iot" -DriverName "ODBC Driver 17 for SQL Server" -Platform "64-bit" -DsnType "System" -SetPropertyValue @("SERVER=$($SERVER)", "Trusted_Connection=Yes", "DATABASE=$($IotDB)")
}
Read-Host -Prompt "Press Enter to exit"