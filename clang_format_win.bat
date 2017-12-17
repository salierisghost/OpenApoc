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