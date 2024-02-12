#include "SessionAwait.h"
#include "../uatypes/UAClient.h"

namespace Jde::Iot{

	α SessionAwait::await_suspend( HCoroutine h )ι->void{
		_client->AddSessionAwait( move(h) );
	}

}