cmake_minimum_required( VERSION ${MIN_REQ_CMAKE_VERSION} )
list( APPEND CMAKE_MODULE_PATH $ENV{JDE_DIR}/Public/build )

include( $ENV{JDE_DIR}/Public/build/functions.cmake )
project( Jde.Opc.Client VERSION ${JDE_VERSION} LANGUAGES CXX )
include( ../../Public/build/dependencies.cmake )
find_package( open62541 REQUIRED )
set( open62541Dir ${open62541_DIR}/../../.. )
cmake_path( ABSOLUTE_PATH open62541Dir NORMALIZE OUTPUT_VARIABLE open62541Dir )

add_subdirectory( $ENV{JDE_DIR}/Framework/source ../../libs/framework )
set( libDir $ENV{JDE_DIR}/Public/libs )
add_subdirectory( ${libDir}/crypto/src ../../libs/crypto/lib )
add_subdirectory( ${libDir}/db/src ../../libs/db/lib )
add_subdirectory( ${libDir}/db/drivers/mysql ../../libs/db/drivers/mysql )
add_subdirectory( ${libDir}/ql ../../libs/ql )
add_subdirectory( ${libDir}/access/src ../../libs/access/lib )
add_subdirectory( ${libDir}/web/client ../../libs/web/client )
add_subdirectory( ${libDir}/web/server ../../libs/web/server )
add_subdirectory( ${libDir}/app/shared ../../libs/app/shared )
add_subdirectory( ${libDir}/app/client ../../libs/app/client )
set( opcCommonDir ${libDir}/opc )
add_subdirectory( ${opcCommonDir}/src ../../libs/opc/lib )

include( $ENV{JDE_DIR}/Public/build/dependencies.cmake )
set( targetName Jde.Opc.Gateway )
project( ${targetName} )

message( ${JSONNET_DIR}/lib )
link_directories( ${JSONNET_DIR}/lib )
link_directories( ${open62541Dir}/lib )
add_definitions( -DUA_ENABLE_ENCRYPTION )

add_executable( ${targetName} )
file( GLOB sources *.cpp types/proto/*.cpp )
file( GLOB headers *.h )
target_sources( ${targetName} PRIVATE ${sources} ${resources} ${headers} )

target_link_libraries( ${targetName} OpenSSL::SSL )
if( WIN32 )
	set_target_properties( ${targetName} PROPERTIES
		VS_DEBUGGER_COMMAND_ARGUMENTS "-c"
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
else()
endif()
include_directories( ${open62541Dir}include )
target_link_libraries( ${targetName} Jde Jde.Access Jde.App.Shared Jde.App.Client Jde.Web.Server Jde.Opc )
target_link_libraries( ${targetName} open62541 )
target_link_libraries( ${targetName} Boost::json )
target_link_libraries( ${targetName} stdc++exp )#for stacktrace

target_precompile_headers(${targetName}
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

add_custom_command( TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink "$ENV{JDE_DIR}/Public/libs/db/config/common-meta.libsonnet" "${CMAKE_CURRENT_SOURCE_DIR}/../config/common-meta.libsonnet" COMMENT "mklink common-meta.libsonnet" )