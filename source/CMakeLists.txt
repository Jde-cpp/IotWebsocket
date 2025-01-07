cmake_minimum_required(VERSION 3.20)
#set( CMAKE_MODULE_PATH $ENV{JDE_DIR}/Public/build )
#message(STATUS "ACMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH $ENV{JDE_DIR}/Public/build )
#message(STATUS "BCMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
#message(STATUS "ACMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
#list( APPEND CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH} )
#message(STATUS "BCMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

include(common)
findBoost()
if( ${VCPKG} )
	SET( CMAKE_TOOLCHAIN_FILE $ENV{REPO_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake  CACHE STRING "Vcpkg toolchain file" )
endif()
project(Jde.IotWebsocket)
include( compiler )
#######################################################
find_package( protobuf REQUIRED )
if( MSVC )
	find_package( OpenSsl REQUIRED )
	include_directories( ${OPENSSL_INCLUDE_DIR} )
	set( installDir $ENV{REPO_DIR}/installed/$(ConfigurationName) )
	include_directories( ${installDir}/include )
	link_directories( ${installDir}/lib )
	link_directories( $ENV{JDE_DIR}/Framework/source/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/web/server/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/iot/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/app/shared/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/web/client/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/crypto/.build/.bin )
	link_directories( $ENV{JDE_DIR}/Public/src/app/client/.build/.bin )
	file(GLOB resources *.rc *.rc )
else()
	#message( a )
	#message(STATUS "CCMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
	#set( CMAKE_MODULE_PATH /usr/share/cmake-3.22/Modules )
	#message(STATUS "DCMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
	find_package( OpenSSL REQUIRED )
	#message( b )
	link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
	link_directories( ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
	find_package(ZLIB)
	find_package(absl REQUIRED)
	find_package(open62541 REQUIRED)
	set( open62541Dir ${open62541_DIR}/../../.. )
	link_directories( ${open62541Dir}/lib )
	link_directories( ${fmt_DIR}/../.. )
	link_directories( ${Protobuf_DIR}/../.. )
	link_directories( $ENV{REPO_DIR}/install/g++-13/asan/jsonnet/lib/ )
endif()

add_executable( ${CMAKE_PROJECT_NAME} )

file(GLOB sources *.cpp types/proto/*.cpp )
file(GLOB headers *.h )

target_sources( ${CMAKE_PROJECT_NAME} PRIVATE ${sources} ${resources} ${headers} )

#######################################################
include_directories( $ENV{JDE_DIR}/Public/include/jde/web/client/proto )
target_link_libraries(${CMAKE_PROJECT_NAME} OpenSSL::SSL)
if( MSVC )
	target_link_libraries( ${CMAKE_PROJECT_NAME} fmt::fmt )
	target_link_libraries( ${CMAKE_PROJECT_NAME} open62541.lib )
	set_target_properties( ${CMAKE_PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_COMMAND_ARGUMENTS "-c"
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
else()
	include_directories( "../../Public" )
	include_directories( "../../Public/jde/log/types/proto" )
	include_directories( ${open62541Dir}/include )
	target_link_libraries( ${CMAKE_PROJECT_NAME} lzma )
	target_link_libraries( ${CMAKE_PROJECT_NAME} ssl )
	target_link_libraries( ${CMAKE_PROJECT_NAME} libcrypto.a )
	target_link_libraries( ${CMAKE_PROJECT_NAME} debug libfmtd.a optimized libfmt.a)
	target_link_libraries( ${CMAKE_PROJECT_NAME} absl::log_internal_message absl::log_internal_check_op absl::cord )
	target_link_libraries( ${CMAKE_PROJECT_NAME} ZLIB::ZLIB )
	target_link_libraries( ${CMAKE_PROJECT_NAME} Boost::json )
	target_link_libraries( ${CMAKE_PROJECT_NAME} jsonnet++ )
	target_link_libraries( ${CMAKE_PROJECT_NAME} Jde Jde.DB libJde.App.ServerLib.a )
	if(	${USE_SHARED_LIBS} )
		target_link_libraries( ${CMAKE_PROJECT_NAME} Jde.Access Jde.App.Shared Jde.QL Jde.Web.Server Jde.Web.Client )
	else()
		target_link_libraries( ${CMAKE_PROJECT_NAME} libJde.Opc.a libJde.App.Client.a libJde.Access.a libJde.App.Shared.a libJde.Web.Server.a libJde.Crypto.a libJde.QL.a libJde.Web.Client.a )
	endif()
endif()
include_directories( $ENV{JDE_DIR}/Public/include/jde/app/shared/proto )
target_link_libraries( ${CMAKE_PROJECT_NAME} protobuf::libprotobuf )
target_link_libraries( ${CMAKE_PROJECT_NAME} libopen62541.a )

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
	<jde/framework.h>
	<jde/opc/usings.h>
	<jde/web/client/exports.h>
	<jde/web/client/proto/Web.FromServer.pb.h>
	<jde/app/shared/exports.h>
	<jde/app/shared/proto/App.FromClient.pb.h>
	<jde/app/shared/proto/App.FromClient.pb.h>
	<jde/app/shared/proto/App.FromServer.pb.h>
	<jde/app/shared/exports.h>
	<jde/app/shared/proto/Common.pb.h>
	<jde/opc/exports.h>
	<jde/opc/types/proto/Opc.Common.pb.h>
	<jde/opc/types/proto/Opc.FromServer.pb.h>
	<jde/opc/types/proto/Opc.FromClient.pb.h>
)
