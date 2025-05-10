{
	logging:{
		defaultLevel: "Information",
		tags: {
			trace:[ "app", "browse", "ql",
				"http.client.write", "http.client.read", "http.server.write", "http.server.read", "socket.client.write", "socket.client.read", "socket.server.write", "socket.server.read",
				"uaNet", "uaSecure", "uaSession", "uaServer", "uaClient", "uaUser", "uaSecurity", "uaEvent", "uaPubSub", "uaDiscovery"
			],
			debug:["settings"],
			information:[
				"iot.read", "iot.monitoring", "iot.browse", "app.processingLoop", "iot.monitoring.pedantic"
			],
			warning:[],
			"error":[],
			critical:[]
		},
		sinks:{
			console:{}
		}
	},
	dbServers: {
			scriptPaths: ["$(JDE_DIR)/IotWebsocket/config/sql/mysql"],
		sync:: false,
		localhost:{
			driver: "$(JDE_BUILD_DIR)/$(JDE_BUILD_TYPE)/libs/db/drivers/mysql/libJde.DB.MySql.so",
			connectionString: "$(JDE_CONNECTION)",
			catalogs: {
				_appCatalog:{
					schemas:{
						_appSchema:{
							access:{
								meta: "$(JDE_DIR)/Public/libs/access/config/access-meta.jsonnet"
							}
						},
					},
				},
				jde: {
					schemas:{
						jde:{ //for sqlserver, test with schema, debug with default schema ie dbo.
							opc:{
								meta: "$(JDE_DIR)/IotWebsocket/config/opcGateway-meta.jsonnet",
								prefix: "opc_"  //test with null prefix, debug with prefix
							}
						}
					}
				}
			}
		}
	},
	credentials:{
		name: "OpcGateway",
		target:: "OpcGateway"
	},
	http:{
		address: null,
		port: 1968,
		threads: 1,
		timeout: "PT30M",
		maxLogLength: 255,
		accessControl: {
			allowOrigin: "*",
			allowMethods: "GET, POST, OPTIONS",
			allowHeaders: "Content-Type, Authorization"
		},
		ssl: {
			certificate:: "{ApplicationDataFolder}/ssl/certs/cert.pem",
			certificateAltName: "DNS:localhost,IP:127.0.0.1",
			certficateCompany:: "Jde-Cpp",
			certficateCountry:: "US",
			certficateDomain:: "localhost",
			privateKey:: "{ApplicationDataFolder}/ssl/private/private.pem",
			publicKey:: "{ApplicationDataFolder}/ssl/public/public.pem",
			dh:: "{ApplicationDataFolder}/certs/dh.pem",
			passcode:: "$(JDE_PASSCODE)"
		}
	},
	workers:{
		drive:{ "threads":  1 },
		alarm:{ "threads":  1 },
		executor: 1
	}
}