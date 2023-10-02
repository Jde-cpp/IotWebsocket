#pragma once

namespace Jde::Iot{
	struct UAClient;
	
	struct AsyncRequest final{
		AsyncRequest( sp<UAClient> pClient )ι;
		~AsyncRequest();
		α Process()ι->void;
	private:
		sp<UAClient> _pClient;
		Handle _handle;
		static atomic<Handle> _index;
	};
}