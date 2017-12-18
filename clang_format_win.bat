clang-format -i version.h
if %ERRORLEVEL% NEQ 0 GOTO :error

@echo off
for /L %%x in (1, 1, 2) do (
	for /R forms %%f in (*.cpp,*.h) do clang-format -i "%%f"
	for /R framework %%f in (*.cpp,*.h) do clang-format -i "%%f"
	for /R game %%f in (*.cpp,*.h) do clang-format -i "%%f"
	for /R library %%f in (*.cpp,*.h) do clang-format -i "%%f"
	for /R tests %%f in (*.cpp,*.h) do clang-format -i "%%f"
	for /R tools %%f in (*.cpp,*.h) do clang-format -i "%%f"
)
del *.TMP /F /S
GOTO :end

:error
echo "clang-format not in path"
GOTO :end

:end