setlocal

rem --- check for trailing backslash
set _test=%WIX:~-1,1%
if NOT %_test% == \ (
  goto OK
)
set WIX=%WIX:~0,-1%

:OK
set WIX_ROOT_DIR=%WIX%
cd ../../
call prep2010.cmd projects build ^
    "-DVERBOSE=1" ^
    "-DPROTOBUF_SRC_ROOT_FOLDER=%PROTOBUF_PATH%" ^
    "-DPROTOBUF_INCLUDE_DIR=%PROTOBUF_PATH%/vsprojects/include"

echo.
endlocal
