::net start Jde.AppServer
::net start Jde.IotWebSocket
cd %ProgramData%\Jde-cpp\IotWeb\
http-server
::net stop Jde.AppServer
::net stop Jde.IotWebSocket