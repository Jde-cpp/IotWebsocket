set ApplicationPath="%ProgramFiles%\jde-cpp\Iot\Jde.IotWebSocket.exe"
cmd /min /C start "" %ApplicationPath% -c
set ApplicationPath="%ProgramFiles%\jde-cpp\AppServer\Jde.AppServer.exe"
cmd /min /C start "" %ApplicationPath% -c
cd %ProgramData%\Jde-cpp\IotWeb\
http-server