
python ../../../tools/gfx2sg.py font.png --bgcolor=1 
REM --preview
REM --compress %* disable due random access! 
REM copy /y "font (palette).bin" "../font (palette).bin"
copy /y "font (tiles).bin" "../font (tiles).bin"
pause
