cmake_minimum_required(VERSION 3.16)
project(Jde.IotWebsocket)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Public/build )
include(common)

#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
find_package(ZLIB)
find_package(absl REQUIRED)
find_package(protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(open62541 REQUIRED)
set( open62541Dir ${open62541_DIR}/../../.. )
link_directories( ${open62541Dir}/lib )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

link_directories( ${fmt_DIR}/../.. )
link_directories( ${Protobuf_DIR}/../.. )

add_executable( ${CMAKE_PROJECT_NAME} )

file(GLOB src_files *.cpp types/proto/*.cpp )
file(GLOB hdr_files  *.h )

target_sources( "${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${src_files}"
        "${hdr_files}")

#######################################################
include_directories( "../../Public" )
include_directories( "../../Public/jde/log/types/proto" )
include_directories( ${open62541Dir}/include )
include_directories( $ENV{JDE_DIR}/Public/jde/app/shared/proto )

target_link_libraries(${CMAKE_PROJECT_NAME} OpenSSL::SSL)
#target_link_libraries( ${CMAKE_PROJECT_NAME} libopen62541.a )
target_link_libraries( ${CMAKE_PROJECT_NAME} lzma )
target_link_libraries( ${CMAKE_PROJECT_NAME} ssl )
target_link_libraries( ${CMAKE_PROJECT_NAME} debug libfmtd.a optimized libfmt.a)
target_link_libraries( ${CMAKE_PROJECT_NAME} protobuf::libprotobuf )
target_link_libraries( ${CMAKE_PROJECT_NAME} absl::log_internal_message absl::log_internal_check_op absl::cord)
target_link_libraries( ${CMAKE_PROJECT_NAME} ZLIB::ZLIB )
target_link_libraries( ${CMAKE_PROJECT_NAME} Jde Jde.App.Shared Jde.App.Client Jde.Crypto Jde.Web.Client Jde.Web.Server Jde.Iot )

target_precompile_headers(${CMAKE_PROJECT_NAME}
	PRIVATE
	<boost/asio.hpp>
	<boost/beast.hpp>
	<boost/asio/strand.hpp>
	<boost/beast/core.hpp>
	<boost/beast/http.hpp>
	<boost/beast/ssl/ssl_stream.hpp>
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
	<jde/opc/usings.h>
	<jde/log/Log.h>
	<jde/App.h>
	"../../Framework/source/coroutine/Awaitable.h"
	"../../Framework/source/io/AsioContextThread.h"
	<jde/coroutine/Task.h>
	<jde/web/server/exports.h>
	<jde/opc/exports.h>
	<jde/app/shared/proto/App.FromClient.pb.h>
	<jde/app/shared/proto/App.FromServer.pb.h>
	<jde/opc/types/proto/Iot.Common.pb.h>
	<jde/opc/types/proto/Iot.FromServer.pb.h>
	<jde/opc/types/proto/Iot.FromClient.pb.h>
#	<jde/opc/types/FromServer.h>
	<jde/opc/uatypes/UAException.h>
	<jde/opc/async/SessionAwait.h>

	"../../Framework/source/DateTime.h"
	"../../Framework/source/db/Database.h"
	"../../Framework/source/db/GraphQL.h"
)
