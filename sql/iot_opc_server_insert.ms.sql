create or alter proc iot_opc_server_insert @name varchar(255),@attributes smallint,@target varchar(255),@description varchar(2047), @certificate_uri varchar(2047), @is_default bit,@url varchar(2047) AS
begin
    if @is_default=1
        update iot_opc_servers set is_default=0;
    insert into iot_opc_servers( name,attributes,created,target,description,certificate_uri,is_default,url )
        values( @name, @attributes, getutcdate(), @target, @description, @certificate_uri, @is_default, @url );
    select SCOPE_IDENTITY();
end
go
drop trigger if exists iot_opc_server_update;
go
create or alter trigger iot_opc_server_update on iot_opc_servers for update AS
begin 
    declare @is_default bit, @id int;
    select @is_default=is_default, @id=id from inserted;
    if @is_default=1
        update iot_opc_servers set is_default=0 where id!=@id;
end
go

