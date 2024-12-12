taskkill /IM foobar2000.exe
Start-Sleep .8
copy ".\Release\foo_bbookmark.dll" $env:APPDATA"\foobar2000-v2\user-components\foo_bbookmark"
start-process foobar2000