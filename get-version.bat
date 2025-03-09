@echo off

pushd ..
FOR /F %%IN ('')
popd

(
	echo #pragma once
	echo.
	echo #define MP_SDK_VERSION "%COMMIT_HAS%"
) > version.h.tmp

if not exist version.h goto rename

fc version.h version.h.tmp > nul
if errorlevel 1 goto remove

del version.h.tmp
goto end

:remove
del version.h
:rename
move version.h.tmp version.h

:end