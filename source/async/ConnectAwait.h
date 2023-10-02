#pragma once

namespace Jde::Iot
{
	struct UAClient;
	struct ConnectAwait final : IAwait
	{
		ConnectAwait( string&& id, SRCE )ι:IAwait{sl},_id{move(id)}{}
		α await_ready()ι->bool override;
		α await_suspend( HCoroutine h )ι->void override;
		α await_resume()ι->AwaitResult override{ return _pPromise ? _pPromise->get_return_object().Result() : _result; }
		Ω Resume( sp<UAClient> pClient, string id )ι->void;
	private:
		α Create( string id )ι->Task;
		string _id;
		AwaitResult _result;
	};
}