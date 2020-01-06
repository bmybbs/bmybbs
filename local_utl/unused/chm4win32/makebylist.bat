for /F %%I in (lists.txt) DO (
call dochm.bat e:\ytht\ytht.net\X\%%I.tgz
del e:\ytht\ann\%%I.chm
move /Y e:\ytht\dest\%%I.chm e:\ytht\ann
del e:\ytht\ytht.net\X\%%I.tgz
echo done %%I>>log.txt
time /t>>log.txt
)