{
	sqlType: "sqlServer",
	logDir: "$(JDE_BUILD_DIR)",
	dbDriver: "$(JDE_BUILD_DIR)/msvc/jde/apps/IotWebsocket/bin/$(JDE_BUILD_TYPE)/Jde.DB.Odbc.dll",
	dbConnectionString: "DSN=jde",
	opc:{
		urn: "urn:JDE-CPP:Kepware.KEPServerEX.V6:UA%20Server",
		url: "opc.tcp://127.0.0.1:49320"
	},
	catalogs: {
		test_opc_debug: {
			schemas:{
				_access:{
					access:{
						meta: "$(JDE_DIR)/Public/libs/access/config/access-meta.jsonnet"
					}
				},
				dbo:{
					opc:{
						meta: "$(JDE_DIR)/IotWebsocket/config/opcGateway-meta.jsonnet",
						prefix: "opc"  //test with null prefix, debug with prefix
					}
				}
			}
		}
	}
}