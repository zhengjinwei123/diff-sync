# diff-sync
windows下差异同步代码，类似rsync

# 联合使用MSBuild 工具 和 diff-sync 一键编译VS项目（例子代码）

``` bat
@set script_path=%~dp0

@set table_compiler=%script_path%\tools\bin\brickred-table-compiler.exe
@set diff_sync=%script_path%\tools\bin\diff-sync.exe
@set table_xml=%script_path%\table.xml
@set output_dir=%script_path%\table-check\table
@set tmp_dir=%script_path%\temp

@if not exist "%table_compiler%" (
    @echo %table_compiler% is missing
    @pause
    @exit /b
)

@if not exist "%diff_sync%" (
    @echo %diff_sync% is missing
    @pause
    @exit /b
)

@if not exist "%table_xml%" (
    @echo %table_xml% is missing
    @pause
    @exit /b
)

@if ["%output_dir%"] == [""] (
    @echo config output_dir is missing
    @pause
    @exit /b
)

@if not exist "%tmp_dir%" (
	@md %tmp_dir%
) else (
	@rd /s /q %tmp_dir%
	@md %tmp_dir%
)

:: generate protocol
@"%table_compiler%" -f "%table_xml%" -l "cpp" -o "%tmp_dir%"

:: diff sync table protocol code
@"%diff_sync%" "%tmp_dir%" "%output_dir%"

@rd /s /q %tmp_dir%

@echo ---------------[generate table protocol success]---------------
@echo ---------------[start build project]---------------------------

@set ms_build=%script_path%\tools\bin\MSBuild.exe
@set sln=%script_path%\table-check.sln

@if not exist "%ms_build%" (
    @echo %ms_build% is missing
    @pause
    @exit /b
)

@if not exist "%sln%" (
    @echo %sln% is missing
    @pause
    @exit /b
)

@"%ms_build%" "%sln%" /property:Configuration=Release;Platform=x86

@echo ---------------[build finished]-------------------------------
@pause
exit /b
```