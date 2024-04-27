$Server = "localhost"
$AppDB = "jde_app"
$IotDB = "jde_iot"

Write-Host "Creating Databases..."
Invoke-Sqlcmd -Query "if not exists(select * from sys.databases where name = '$($AppDB)') create database $($AppDB)" -ServerInstance "$($Server)"
Invoke-Sqlcmd -Query "if not exists(select * from sys.databases where name = '$($IotDB)') create database $($IotDB)" -ServerInstance "$($Server)"
Write-Host "Granting DB Owner to System account..."
Invoke-Sqlcmd -Query "use $($AppDB);exec sp_addrolemember 'db_owner', 'NT AUTHORITY\System'; ALTER USER [NT AUTHORITY\System] WITH DEFAULT_SCHEMA=dbo;use $($IotDB);exec sp_addrolemember 'db_owner', 'NT AUTHORITY\System';ALTER USER [NT AUTHORITY\System] WITH DEFAULT_SCHEMA=dbo;"

Write-Host "Creating ODBC Datasources..."
if ( (Get-OdbcDsn -Name "jde_app" -ErrorAction Ignore) -eq $null){
  Add-OdbcDsn -Name "jde_app" -DriverName "ODBC Driver 17 for SQL Server" -Platform "64-bit" -DsnType "System" -SetPropertyValue @("SERVER=$($SERVER)", "Trusted_Connection=Yes", "DATABASE=$($AppDB)")
}
if ( (Get-OdbcDsn -Name "jde_iot" -ErrorAction Ignore) -eq $null){
  Add-OdbcDsn -Name "jde_iot" -DriverName "ODBC Driver 17 for SQL Server" -Platform "64-bit" -DsnType "System" -SetPropertyValue @("SERVER=$($SERVER)", "Trusted_Connection=Yes", "DATABASE=$($IotDB)")
}
Write-Host "Registering Services..."
& "$Env:Programfiles\Jde-cpp\iot\Jde.IotWebSocket.exe"  -install
& "$Env:Programfiles\Jde-cpp\AppServer\Jde.AppServer.exe"  -install

Read-Host -Prompt "Press Enter to exit"