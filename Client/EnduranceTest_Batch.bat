@echo off
SET SERVER_IP=172.20.10.4
SET SERVER_PORT=5000
SET DATA_FILE=katl-kefd-B737-700.txt

SET /A "index = 1"
SET /A "count = 100"

:while
@echo %time%
	:spawnloop
	if %index% leq %count% (
		START /MIN Release\Client.exe %SERVER_IP% %SERVER_PORT% %DATA_FILE%
		SET /A index = %index% + 1
		@echo %index%
		goto :spawnloop
	)
	timeout 250 > NUL
	SET /A index = 1
	goto :while
