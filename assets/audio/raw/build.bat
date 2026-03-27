REM music
for %%i in (stones shop dungeon over lordbrit intro) do (
	python ..\..\tools\midi2vgm.py %%i.mid %%i.vgm
    	call encode %%i
) 
REM sfx
for %%i in (fall hit blocked trap gold) do (
	python ..\..\tools\midi2vgm.py %%i.mid  %%i.vgm
	call encode %%i
) 
pause
