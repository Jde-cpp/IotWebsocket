local common = import 'common-meta.libsonnet';
{
	local tables = self.tables,
	tables:{
		servers:{
			columns: {
				is_default: common.types.bit+{i:101},
				certificate_uri: common.types.uri+{nullable:true, length:2048,i:102},
				url: common.types.uri+{nullable:true, length:2048, i:103}
			}+common.targetColumns,
			customInsertProc:true,
			naturalKeys: common.targetNKs
		},
	}
}