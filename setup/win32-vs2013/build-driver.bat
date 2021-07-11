set CURDIR=%~dp0
cd %CURDIR%

call create_solution.bat
echo create_solution over

echo please SET VS2013_HOME=D:\software\Visual Studio\Visual Studio 2013 Ultimate
set path=%VS2013_HOME%\VC\bin\x86_amd64;%path%
call vcvarsx86_amd64.bat

echo ready to build solution...
call devenv ..\..\cmake-temp\eview-server.sln /build "RelWithDebInfo|x64" /useenv


