{
	logging:{
		defaultLevel: "Information",
		tags: {
			Trace:[ "app", "browse",
				"http.client.write", "http.client.read", "http.server.write", "http.server.read", "socket.client.write", "socket.client.read", "socket.server.write", "socket.server.read",
				"uaNet", "uaSecure", "uaSession", "uaServer", "uaClient", "uaUser", "uaSecurity", "uaEvent", "uaPubSub", "uaDiscovery"
			],
			Debug:["settings"],
			Information:[
				"iot.read", "iot.monitoring", "iot.browse", "app.processingLoop", "iot.monitoring.pedantic"
			],
			Warning:[],
			Error:[],
			Critical:[]
		},
		sinks:{
			console:{}
		}
	},
	dbServers: {
		scriptPath: "$(JDE_DIR)/IotWebsocket/config/sql/mysql/opcClient",
		localhost:{
			driver: "$(JDE_DIR)/bin/asan/libJde.MySql.so",
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
				jde_opc_test: {
					schemas:{
						jde:{ //for sqlserver, test with schema, debug with default schema ie dbo.
							opc:{
								meta: "$(JDE_DIR)/IotWebsocket/config/IotWebSocketMeta.jsonnet",
								prefix: "opc_"  //test with null prefix, debug with prefix
							}
						}
					}
				}
			}
		}
	},
	credentials:{
		name: "IotServer",
		target:: "IotServer"
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
		Alarm:{ "threads":  1 },
		executor: 1
	}
}