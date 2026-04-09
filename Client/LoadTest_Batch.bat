@echo off
SET SERVER_IP=192.168.1.100
SET SERVER_PORT=5000
SET DATA_FILE=katl-kefd-B737-700.txt

SET /A "count = 25"

:restart
SET /A "index = 1"
@echo Spawning wave of %count% clients...

:while
if %index% leq %count% (
	START /MIN Client.exe %SERVER_IP% %SERVER_PORT% %DATA_FILE%
	SET /A index = %index% + 1
	@echo %index%
	goto :while
)

@echo Wave complete. Restarting...
goto :restart
