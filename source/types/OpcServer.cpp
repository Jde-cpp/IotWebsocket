#include "OpcServer.h"
#include "../../Framework/source/db/types/WhereClause.h"
namespace Jde::Iot
{
	OpcServer::OpcServer( const DB::IRow& r )ι:
		Id{ r.GetUInt32(0) },
		Address{ r.GetString(1) },
		IsDefault{ r.GetBit(2) },
		Name{ r.GetString(3) },
		Target{ r.GetString(4) }
	{}


	α Load(optional<string> id, HCoroutine h, bool includeDeleted=false)->Task
	{
		DB::WhereClause where{ includeDeleted ? "" : "deleted is null" };
		vector<DB::object> params;
		if( id )
		{
			if( id->size() )
			{
				where << "name=?";
				params.push_back( *id );
			}
			else
				where << "is_default=true";
		}

		auto y = (co_await DB::SelectCo<vector<OpcServer>>(format("select id, address, is_default, name, target from opc_servers {}", where.Move()), params, []( vector<OpcServer>& result, const DB::IRow& r )
		{
			result.push_back(OpcServer{r});
		} )).UP<vector<OpcServer>>();
		auto& result = h.promise().get_return_object();
		if( id )
			result.SetResult( y->size() ? mu<OpcServer>(move((*y)[0])) : up<OpcServer>{} );
		else
			result.SetResult( move(y) );
		h.resume();
	}
	α OpcServer::Select()ι->AsyncAwait
	{
		return AsyncAwait{ [](HCoroutine h)->Task {return Load(nullopt,h);} };
	}
	α OpcServer::Select( string id )ι->AsyncAwait
	{
		return AsyncAwait{ [&id](HCoroutine h)->Task {return Load(move(id),h);} };
	}
}