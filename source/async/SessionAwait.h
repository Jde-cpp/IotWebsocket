﻿#pragma once

namespace Jde::Iot{
	struct UAClient;
	struct SessionAwait final : IAwait{
		SessionAwait( sp<UAClient> client, SRCE )ι:IAwait{sl}, _client{move(client)}{}
		α await_suspend( HCoroutine h )ι->void override;
		Ω Trigger( sp<UAClient>&& pClient )ι->void;
	private:
		sp<UAClient> _client;
	};
	Ξ AwaitSessionActivation( sp<UAClient> p, SRCE )ι->SessionAwait{ return SessionAwait{p, sl}; }
}