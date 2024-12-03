New-Item -ItemType Directory -Path ./newRelease/x64

copy ..\x64\ReleaseFoobar\foo_bbookmark.dll .\newRelease\x64
copy ..\Release\foo_bbookmark.dll .\newRelease

7z a newRelease.zip .\newRelease\*

mv newRelease.zip newRelease.fb2k-component