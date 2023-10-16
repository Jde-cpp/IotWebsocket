cmake_minimum_required(VERSION 3.16)
project(Jde.IotWebsocket)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Framework/cmake )
include(SetUpConfigurations)
#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
link_directories( ../../open62541/${CMAKE_BUILD_TYPE}/bin )
find_package(ZLIB)

add_executable( Jde.IotWebsocket Main.cpp Rest.cpp
	async/AsyncRequest.cpp async/ConnectAwait.cpp
	types/OpcServer.cpp
	uatypes/Browse.cpp uatypes/Logger.cpp uatypes/Value.cpp uatypes/UAClient.cpp )
#######################################################
include_directories( "../../Public" )
include_directories( "../../Public/jde/log/types/proto" )
include_directories( "../../open62541/include" )
include_directories( "../../open62541/plugins/include" )
include_directories( "../../open62541/${CMAKE_BUILD_TYPE}/src_generated" )

target_link_libraries( Jde.IotWebsocket libopen62541.a )
target_link_libraries( Jde.IotWebsocket lzma )
target_link_libraries( Jde.IotWebsocket protobuf )
target_link_libraries( Jde.IotWebsocket ssl )
target_link_libraries( Jde.IotWebsocket ZLIB::ZLIB )
target_link_libraries( Jde.IotWebsocket Jde )
target_link_libraries( Jde.IotWebsocket Jde.Ssl )
target_link_libraries( Jde.IotWebsocket Jde.Web )
target_link_libraries( Jde.IotWebsocket Jde.XZ )

#target_link_libraries(Jde.IotWebsocket Threads::Threads)
#target_link_libraries(Jde.IotWebsocket dl)
#target_link_libraries( Jde.IotWebsocket crypto )

#target_link_libraries(Jde.IotWebsocket stdc++)
#target_link_libraries(Jde.IotWebsocket boost_system)
#target_link_libraries(Jde.IotWebsocket boost_thread)
#target_link_libraries(Jde.IotWebsocket mysqlcppconn8)
#target_link_libraries(Jde.IotWebsocket Jde.MySql)
#######################################################
target_precompile_headers(Jde.IotWebsocket
	PRIVATE
	<boost/lexical_cast.hpp>
	<boost/uuid/uuid.hpp>
	<boost/uuid/uuid_io.hpp>
	<open62541/client_config_default.h>
	<open62541/client_highlevel.h>
	<open62541/client_highlevel_async.h>
	<open62541/plugin/log_stdout.h>
	<jde/TypeDefs.h>
	<jde/Log.h>
	<jde/App.h>
	<jde/Str.h>
	"types/proto/IotFromServer.pb.h"
	"types/proto/IotFromClient.pb.h"
	"types/FromServer.h"
	"uatypes/UAException.h"
	"../../Framework/source/DateTime.h"
	"../../Framework/source/coroutine/Alarm.h"
	"../../Framework/source/db/Database.h"
	"../../Framework/source/db/GraphQL.h"
	"../../Framework/source/io/AsioContextThread.h"
	"../../Framework/source/io/Socket.h"
	"../../Public/src/web/RestServer.h"
)
