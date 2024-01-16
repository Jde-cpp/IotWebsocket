#pragma once

namespace Jde::Iot{
	struct CreateMonitoredItemsRequest : UA_CreateMonitoredItemsRequest{
		CreateMonitoredItemsRequest( flat_set<NodeId>&& nodes )ι;
		CreateMonitoredItemsRequest( CreateMonitoredItemsRequest&& x )ι;
		~CreateMonitoredItemsRequest(){ UA_CreateMonitoredItemsRequest_clear( this ); }
		α operator=( CreateMonitoredItemsRequest&& )->UA_CreateMonitoredItemsRequest;
	};

	inline CreateMonitoredItemsRequest::CreateMonitoredItemsRequest( flat_set<NodeId>&& nodes )ι{
		UA_CreateMonitoredItemsRequest_init( this );
		itemsToCreateSize = nodes.size();
		itemsToCreate = (UA_MonitoredItemCreateRequest*)UA_Array_new( itemsToCreateSize, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST] );
		uint i=0;
		for( auto& n : nodes ){
			auto& item = itemsToCreate[i++];
			item.itemToMonitor.nodeId = n.nodeId;
			item.itemToMonitor.attributeId = UA_ATTRIBUTEID_VALUE;
			item.monitoringMode = UA_MONITORINGMODE_REPORTING;
			item.requestedParameters.samplingInterval = 500.0;
			item.requestedParameters.queueSize = 1;
			item.requestedParameters.discardOldest = true;
		}
	}
	inline CreateMonitoredItemsRequest::CreateMonitoredItemsRequest( CreateMonitoredItemsRequest&& x )ι:
		UA_CreateMonitoredItemsRequest{ x }{
		UA_CreateMonitoredItemsRequest_init( &x );
	}

	Ξ CreateMonitoredItemsRequest::operator=( CreateMonitoredItemsRequest&& )->UA_CreateMonitoredItemsRequest{
		UA_CreateMonitoredItemsRequest r = *this;
		UA_CreateMonitoredItemsRequest_init( this );
		return r;
	}
}