@echo off
del log.txt
echo start at >>log.txt
time /t>>log.txt

wget -r -N -l 1 -nr http://ytht.net/Ytht.Net/bbsx/
del lists.txt
for %%I in (e:\ytht\ytht.net\X\*.tgz) DO (
echo %%~nI>>lists.txt
)
call makebylist.bat
echo end at>>log.txt
time /t>>log.txt
@echo on