drop procedure if exists iot_opc_server_insert;
GO
#DELIMITER $$
  CREATE PROCEDURE iot_opc_server_insert( _name varchar(255),_attributes smallint,_target varchar(255),_description varchar(2047), _certificate_uri varchar(2047), _is_default bit,_url varchar(2047) )
  begin
    if( _is_default ) then
      update iot_opc_servers set is_default=0;
    end if;
  	insert into iot_opc_servers( name,attributes,created,target,description,certificate_uri, is_default, url )
  		values( _name,_attributes,CURRENT_TIMESTAMP(),_target,_description, _certificate_uri, _is_default,_url );
  	select LAST_INSERT_ID();
  end
#$$
#DELIMITER ;
GO
drop trigger if exists iot_opc_server_update;
GO
#DELIMITER $$
create trigger iot_opc_server_update before update on iot_opc_servers for each row
  if( NEW.is_default and (select count(*) from iot_opc_servers where id!=NEW.id and is_default)>0 ) then
    set NEW.is_default = 0;
  end if;
#$$
