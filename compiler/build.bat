@echo off

SET OPTS=-FC -GR- -EHa- -nologo -Zi
SET CODE_HOME=%cd%
pushd .
cl %OPTS% %CODE_HOME%\main.c -Fe.
popd
