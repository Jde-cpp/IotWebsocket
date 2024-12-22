#!/bin/bash
branch=${1:-master}
clean=${2:-0};
shouldFetch=${3:-0};
tests=${4:-1}

[[ "$branch" = master ]] && branch2=main || branch2="$branch"
echo build-iotwebsocket.sh branch: $branch2 clean: $clean shouldFetch: $shouldFetch tests: $tests;
scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
if [ -z $JDE_BASH ]; then JDE_BASH=$scriptDir/jde; fi;
if [ -z $REPO_DIR ]; then REPO_DIR=$scriptDir/libraries; fi;

if [ ! -d $JDE_BASH ]; then mkdir jde; fi;
if [ ! -d $JDE_BASH/Framework ]; then cd $JDE_BASH; git clone -b $branch --single-branch https://github.com/Jde-cpp/Framework.git; cd ..; fi;
if ! source $JDE_BASH/Framework/scripts/common-error.sh; then exit 1; fi;
source $JDE_BASH/Framework/scripts/source-build.sh;
$JDE_BASH/Framework/scripts/framework-build.sh $branch $clean $shouldFetch $tests;

if [ ! windows -a ! -z "$install" ]; then #https://www.open62541.org/doc/open62541-1.3.pdf
	sudo apt-get install git build-essential gcc pkg-config cmake python
	sudo apt-get install cmake-curses-gui # for the ccmake graphical interface
	sudo apt-get install libmbedtls-dev # for encryption support
	sudo apt-get install check libsubunit-dev # for unit tests
	sudo apt-get install python-sphinx graphviz # for documentation generation
	sudo apt-get install python-sphinx-rtd-theme # documentation style
fi;

echo -------------Crypto------------
cd $JDE_BASH/Public/src/crypto;
	buildCMake Jde.Crypto;

echo -------------Web.Client------------
cd $JDE_BASH/Public/src/web/client;
	buildCMake Jde.Web.Client;

echo -------------App.Shared------------
cd $JDE_BASH/Public/src/app/shared;
	buildCMake Jde.App.Shared;

echo -------------App.Client------------
cd $JDE_BASH/Public/src/app/client;
	buildCMake Jde.App.Client;

exit 0;

echo -------------Crypto------------
cd $JDE_BASH/Public/src/crypto;
buildCMake Jde.Crypto;
if [ $tests -eq 1 ]; then
	cd $JDE_BASH/Public/tests/crypto;
	buildCMake Jde.Crypto.Tests;
fi;

echo -------------Web.Client------------
cd $JDE_BASH/Public/src/web/client;
buildCMake Jde.Web.Client;

echo -------------App.Shared------------
cd $JDE_BASH/Public/src/app/shared;
buildCMake Jde.App.Shared;

echo -------------App.Client------------
cd $JDE_BASH/Public/src/app/client;
buildCMake Jde.App.Client;

echo -------------Web.Server------------
cd $JDE_BASH/Public/src/web/server;
buildCMake Jde.Web.Server;
if [ $tests -eq 1 ]; then
	cd $JDE_BASH/Public/tests/web;
	buildCMake Jde.Web.Tests;
fi;

echo -------------open62541-------------
cd $REPO_DIR;
if [ ! -d open62541 ]; then git clone https://github.com/Jde-cpp/open62541.git; fi;
cd open62541;
if [ $shouldFetch -eq 1 ]; then
	git pull > /dev/null;
fi;
if windows; then
	export CMAKE_INSTALL_PREFIX=$REPO_DIR/installed;
else
	export CMAKE_INSTALL_PREFIX=$REPO_DIR/install/CXX/$CMAKE_BUILD_TYPE;
fi;
buildCMake open62541;
cd $REPO_DIR/open62541/.build;
if windows; then
	cmake.exe -DBUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX/debug -P cmake_install.cmake;
else
	cmake.exe -DBUILD_TYPE=debug -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX/asan -P cmake_install.cmake;
fi;
cmake.exe -DBUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX/RelWithDebInfo -P cmake_install.cmake;

echo ----------------iot----------------
cd $JDE_BASH/Public/src/iot;
buildCMake Jde.Iot;
if [ $tests -eq 1 ]; then
	cd $JDE_BASH/Public/tests/iot;
	buildCMake Jde.Iot.Tests;
fi;

if windows; then
	cd $JDE_BASH;
	fetchDefault master Odbc 0;
	buildCMake Jde.DB.Odbc 0 DB.Odbc;
fi;

cd $JDE_BASH;
fetchDefault main IotWebsocket 0;
buildCMake Jde.IotWebsocket 0 Jde.IotWebsocket.exe;

cd $JDE_BASH;moveToDir web;
fetch main IotSite;
./scripts/setup.sh

exit 0;

