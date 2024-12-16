#!/bin/bash
branch=${1:-master}
clean=${2:-0};
shouldFetch=${3:-1};
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

echo ----------------Web----------------
findProtoc;
cd $JDE_BASH/Public/src/web;
cd proto;
file=FromServer;
if [[ ! -f $file.pb.h  || $shouldFetch -eq 1 ]]; then
	protoc --cpp_out dllexport_decl=JDE_WEB_EXPORTS:. -I. $file.proto;
	sed -i -e 's/JDE_WEB_EXPORTS/ΓW/g' $file.pb.h;
	sed -i '1s/^/\xef\xbb\xbf/' $file.pb.h;
	sed -i -e 's/PROTOBUF_CONSTINIT ExceptionDefaultTypeInternal/ExceptionDefaultTypeInternal/g' $file.pb.cc;
fi;
cd ..;
build Web;
echo -------------open62541-------------
cd $REPO_DIR;
if [ ! -d open62541 ]; then git clone https://github.com/Jde-cpp/open62541.git; fi;
cd open62541;
if [ $shouldFetch -eq 1 ]; then
	git pull > /dev/null;
	if [ -f build/.bin/debug/open62541.lib ]; then rm build/.bin/debug/open62541.lib; fi;
	if [ -f build/.bin/release/open62541.lib ]; then rm build/.bin/release/open62541.lib; fi;
fi;
# Use pre-made sln
# if [ ! -f open62541.sln ]; then
# 	echo open62541.sln - `pwd`
# 	moveToDir build;
# 	CMAKE_CXX_FLAGS="-std=c++20 -wd"4068" -wd"5105" -Zc:__cplusplus -std:c17";# -TP
# 	cmake -DUA_LOGLEVEL=100 -DUA_ENABLE_ENCRYPTION_OPENSSL=ON ..;#-DUA_THREADSAFE=100
# fi;
stage=$JDE_BASH/Public/stage;
if [[ ! -f $stage/debug/open62541.lib || ! -f $stage/release/open62541.lib ]]; then
	cd build;
	build open62541 0 open62541.lib;
fi;
echo REPO_BASH=$REPO_BASH
cd $JDE_BASH;
fetchDefault IotWebsocket;
echo -------------------proto-------------------------;
SOURCE_DIR=$JDE_BASH/Public/jde/iot/types/proto;
cd $SOURCE_DIR;
moveToDir google;
moveToDir protobuf;
mklink "duration.proto" $PROTOBUF_INCLUDE;
mklink "timestamp.proto" $PROTOBUF_INCLUDE;
cd ../..;
mklink "FromServer.proto" $JDE_BASH/Public/src/web/proto;
mklink "FromServer.pb.h" $JDE_BASH/Public/src/web/proto;
file=IotFromServer;
if [[ ! -f $file.pb.h || $shouldFetch -eq 1 ]]; then
	protoc --cpp_out dllexport_decl=Jde_Iot_EXPORTS:. $file.proto;
	sed -i -e 's/Jde_Iot_EXPORTS/ΓI/g' $file.pb.h;
	sed -i '1s/^/\xef\xbb\xbf/' $file.pb.h;
	sed -i -e 's/PROTOBUF_CONSTINIT NodeValuesDefaultTypeInternal/NodeValuesDefaultTypeInternal/g' $file.pb.cc;
fi;
file=IotFromClient;
if [[ ! -f $file.pb.h  || $shouldFetch -eq 1 ]]; then
	protoc --cpp_out dllexport_decl=Jde_Iot_EXPORTS:. $file.proto;
	sed -i -e 's/Jde_Iot_EXPORTS/ΓI/g' $file.pb.h;
	sed -i '1s/^/\xef\xbb\xbf/' $file.pb.h;
	sed -i -e 's/PROTOBUF_CONSTINIT SubscribeDefaultTypeInternal/SubscribeDefaultTypeInternal/g' $file.pb.cc;
	sed -i -e 's/PROTOBUF_CONSTINIT UnsubscribeDefaultTypeInternal/UnsubscribeDefaultTypeInternal/g' $file.pb.cc;
fi;
file=IotCommon;
if [[ ! -f $file.pb.h  || $shouldFetch -eq 1 ]]; then
	protoc --cpp_out dllexport_decl=Jde_Iot_EXPORTS:. $file.proto;
	sed -i -e 's/Jde_Iot_EXPORTS/ΓI/g' $file.pb.h;
	sed -i '1s/^/\xef\xbb\xbf/' $file.pb.h;
	sed -i -e 's/PROTOBUF_CONSTINIT ExpandedNodeIdDefaultTypeInternal/ExpandedNodeIdDefaultTypeInternal/g' $file.pb.cc;
fi;

cd $JDE_BASH/Public/src/iot/types/proto;
mv $SOURCE_DIR/IotFromServer.pb.cc .;
mklink IotFromServer.pb.h $SOURCE_DIR;
mv $SOURCE_DIR/IotFromClient.pb.cc .;
mklink IotFromClient.pb.h $SOURCE_DIR;
mv $SOURCE_DIR/IotCommon.pb.cc .;
mklink IotCommon.pb.h $SOURCE_DIR;

cd $JDE_BASH;

build IotWebsocket 0 Jde.IotWebsocket.exe;

cd $JDE_BASH;moveToDir web;
fetch IotSite;
