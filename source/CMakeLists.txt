cmake_minimum_required(VERSION 3.16)
project(Jde.IotWebsocket)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Public/build )
include(common)

#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
find_package(ZLIB)
find_package(absl REQUIRED)
find_package(protobuf REQUIRED)
find_package(open62541 REQUIRED)
set( open62541Dir ${open62541_DIR}/../../.. )
link_directories( ${open62541Dir}/lib )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

link_directories( ${fmt_DIR}/../.. )
link_directories( ${Protobuf_DIR}/../.. )

add_executable( ${CMAKE_PROJECT_NAME} Main.cpp Rest.cpp Socket.cpp
	async/AsyncRequest.cpp async/Attributes.cpp async/CreateSubscriptions.cpp async/ConnectAwait.cpp async/DataChanges.cpp async/SessionAwait.cpp async/SetMonitoringMode.cpp async/Write.cpp
	types/MonitoringNodes.cpp	types/OpcServer.cpp
	uatypes/Browse.cpp uatypes/Logger.cpp uatypes/Node.cpp uatypes/Value.cpp uatypes/UAClient.cpp
	types/proto/IotCommon.pb.cc types/proto/IotFromClient.pb.cc types/proto/IotFromServer.pb.cc )
#######################################################
include_directories( "../../Public" )
include_directories( "../../Public/jde/log/types/proto" )
include_directories( ${open62541Dir}/include )

target_link_libraries( ${CMAKE_PROJECT_NAME} libopen62541.a )
target_link_libraries( ${CMAKE_PROJECT_NAME} lzma )
target_link_libraries( ${CMAKE_PROJECT_NAME} ssl )
target_link_libraries( ${CMAKE_PROJECT_NAME} debug libfmtd.a optimized libfmt.a)
target_link_libraries( ${CMAKE_PROJECT_NAME} protobuf::libprotobuf )
target_link_libraries( ${CMAKE_PROJECT_NAME} absl::log_internal_message absl::log_internal_check_op absl::cord)
target_link_libraries( ${CMAKE_PROJECT_NAME} ZLIB::ZLIB )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde.Crypto )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde.Ssl )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde.Web )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde.XZ )

target_precompile_headers(${CMAKE_PROJECT_NAME}
	PRIVATE
	<boost/asio.hpp>
	<boost/beast.hpp>
	<boost/beast/websocket/stream.hpp>
	<boost/exception/diagnostic_information.hpp>
	<boost/lexical_cast.hpp>
	<boost/unordered/concurrent_flat_map.hpp>
	<boost/unordered/concurrent_flat_set.hpp>
	<boost/uuid/uuid.hpp>
	<boost/uuid/uuid_io.hpp>
	<open62541/client_config_default.h>
	<open62541/client_highlevel.h>
	<open62541/client_highlevel_async.h>
	<open62541/plugin/log_stdout.h>
	<open62541/client_subscriptions.h>
	"TypeDefs.h"
	<jde/Log.h>
	<jde/App.h>
	<jde/Str.h>
	"../../Framework/source/DateTime.h"
	"../../Public/src/web/Exports.h"
	"../../Public/src/web/proto/FromServer.pb.h"
	"types/proto/IotFromServer.pb.h"
	"types/proto/IotFromClient.pb.h"
	"../../Framework/source/coroutine/Awaitable.h"
	"async/SessionAwait.h"
	"types/FromServer.h"
	"uatypes/UAException.h"
	"../../Framework/source/coroutine/Alarm.h"
	"../../Framework/source/db/Database.h"
	"../../Framework/source/db/GraphQL.h"
	"../../Framework/source/io/AsioContextThread.h"
	"../../Framework/source/io/Socket.h"
	"../../Public/src/web/RestServer.h"
)
