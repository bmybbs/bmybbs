cscript makehhp.wsf /z:%1 /c /f /e
IF exist %~n1\index.html cscript makehhp.wsf /d:e:\ytht\%~n1\ /c /e
