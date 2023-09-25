#pragma once

namespace Jde::Iot
{
	namespace Rest
	{
		α DoAccept()ι->void;
	}
	using namespace Web::Rest;
	struct Session : ISession, std::enable_shared_from_this<Session>
	{
		Session( tcp::socket&& socket ): ISession{move(socket)}{}
		virtual ~Session(){}
		α HandleRequest( string&& target, flat_map<string,string>&& params, Request&& req )ι->void override;
		α MakeShared()ι->sp<ISession> override{ return shared_from_this(); }
	};
}
