/***************************************************************************

  Cldap.c

  Ldap routines

  (c) 2005 Carlo Sorda <gambas@users.sourceforge.net><csorda@libero.it>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CLDAP_C

#include "Cldap.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "main.h"
#include "gambas.h"
#include "gb_common.h"


void LDAP_init(void)
{
}
void LDAP_exit(void)
{
  
}

/***************************************************************************

 ldap

***************************************************************************/

#define THIS ((CLDAP *)_object)

BEGIN_METHOD(CLDAP_new,GB_STRING hostName)
	if ( (THIS->ld = (LDAP *)ldap_init(GB.ToZeroString(ARG(hostName)),LDAP_PORT)) == 0)
   	GB.Error("ldap_init error!"); 
	
	if (THIS->ld == NULL)
		printf("Error ld is NULL.\n");
	else
		printf("ldap_init is correct.\n");		

END_METHOD

BEGIN_METHOD_VOID(CLDAP_free)
	
	if (THIS->ld != NULL)
	{
		int rc = ldap_unbind(THIS->ld);
		
		if ( rc != LDAP_SUCCESS ) 
			GB.Error(ldap_err2string( rc ) );
// 		else
// 			THIS->ld	= NULL;
	}	
END_METHOD

BEGIN_METHOD(CLDAP_init,GB_STRING hostName;GB_INTEGER port)
	
	bool rc = TRUE;
	
	if ( (THIS->ld = (LDAP *)ldap_init(GB.ToZeroString(ARG(hostName)),VARG(port))) == NULL)
	{
		GB.Error("ldap_init error!");
		rc = FALSE;  
	}
	

	GB.ReturnBoolean(rc);		

END_METHOD
BEGIN_METHOD(CLDAP_bind,GB_STRING dn;GB_STRING pw)
		
	int rc = ldap_simple_bind_s( 
					THIS->ld,
					GB.ToZeroString(ARG(dn)),
					GB.ToZeroString(ARG(pw))
									  );
	if ( rc != LDAP_SUCCESS ) 
		GB.Error(ldap_err2string( rc ));

	GB.ReturnInteger(rc);
	
END_METHOD
BEGIN_METHOD_VOID(CLDAP_unbind)
	
	int rc;
	rc = ldap_unbind(THIS->ld);
	
	if ( rc != LDAP_SUCCESS ) 
		GB.Error(ldap_err2string( rc ) );
	else
		THIS->ld	= NULL;
	
	GB.ReturnInteger(rc);
	
END_METHOD
		
BEGIN_METHOD(CLDAP_search,GB_STRING BaseDn;GB_INTEGER Scope;GB_STRING Filter;GB_STRING sort)		
	

	LDAPMessage   	*result,*msg;
	BerElement     *ber;	
	int           	scope_,rc,n,msgtype;
	char           *dn;
	char 				*a;
	char           **vals;
	char				*sortby_;
	
	GB_ARRAY array;
	scope_ = VARG(Scope);
	sortby_ = GB.ToZeroString(ARG(sort));
  
	if (THIS->ld != NULL)
	{
		rc = ldap_search_ext_s( THIS->ld, GB.ToZeroString(ARG(BaseDn)), scope_, GB.ToZeroString(ARG(Filter)), NULL, 0, 
									NULL, NULL, NULL, 0, &result );
	
		if ( rc == LDAP_SUCCESS ) 
		{
	
			if ( strlen(sortby_) > 0 )
			{
				/* Sort the results by room number, using strcasecmp */
				rc = ldap_sort_entries( THIS->ld, &result, sortby_, strcasecmp ); 
						
				if (rc == LDAP_SUCCESS)
					GB.Error(ldap_err2string(rc) );
			}	
			
			if  ( rc == LDAP_SUCCESS)
			{
				n = ldap_count_entries(THIS->ld, result);
						
				GB.Array.New(&array, GB_T_OBJECT, n);
				
				int i=0;  
				int cont=0;
				
				//	a search reference, or the final result of the search operation. 
				for ( msg = ldap_first_message( THIS->ld, result ); msg != NULL; msg = ldap_next_message( THIS->ld, msg ) ) 
				{
					// Determine what type of message was sent from the server. 
					msgtype = ldap_msgtype( msg );
					switch( msgtype ) 
					{
						CLDAPSEARCHENTRY *object;
						// If the result was an entry found by the search, get and print the
						//	attributes and values of the entry. 
						case LDAP_RES_SEARCH_ENTRY:
				
							GB.New((void **)&object, GB.FindClass("LDAPSearchEntry"), NULL, NULL);
							GB_COLLECTION aCollection;
							GB.Collection.New(&aCollection, GB_COMP_BINARY);
								
							// Get and print the DN of the entry. 
							if (( dn = ldap_get_dn( THIS->ld, result )) != NULL ) {
								GB.NewString(&object->dn, dn, 0);
								ldap_memfree( dn );
							}
							
							// Iterate through each attribute in the entry. i
							for ( a = ldap_first_attribute( THIS->ld, result, &ber ); a != NULL; a = ldap_next_attribute( THIS->ld, result, ber ) ) 
							{
								// Get and print all values for each attribute. 
								if (( vals = (char **)ldap_get_values( THIS->ld, result, a )) != NULL ) 
								{
									for ( i = 0; vals[ i ] != NULL; i++ ) 
									{
										//----------------------
										//Add in the collection
										//----------------------
										GB_VARIANT aValue;
										const char*data;
				
										GB.NewString((char**)&data,vals[ i ],0);
										aValue.type = GB_T_VARIANT;
										aValue.value.type = GB_T_STRING;
										aValue.value._string.value = (char* )data;
				
										GB.Collection.Set(aCollection,(const char*)a,strlen(a),&aValue);
									}
									
									ldap_value_free( vals );
								}
									
								ldap_memfree( a );
							}
							object->attributes = aCollection;
							GB.Ref(object);
							*((CLDAPSEARCHENTRY **)GB.Array.Get(array, cont++)) = object;
							
							if ( ber != NULL ) {
								ber_free( ber, 0 );
							}
							break;
						
						default:
							break;
					}//End switch
				}
			}
			
		}
		}
  
	ldap_msgfree( result );

	GB.ReturnObject(array);
		  
END_METHOD
				
	
GB_DESC CLDAPDesc[] = 
{
	GB_DECLARE("Ldap",sizeof(CLDAP)),
    
	//GB_METHOD("_new",NULL,CLDAP_new,"[(HostName)s]"),
	GB_METHOD("_new",NULL,CLDAP_new,NULL),
	GB_METHOD("_free",NULL,CLDAP_free,NULL),
	
// # LDAP_SUCCESS if successful.
// # LDAP_PARAM_ERROR if any of the arguments are invalid.
// # LDAP_ENCODING_ERROR if an error occurred when BER-encoding the request.
// # LDAP_SERVER_DOWN if the LDAP server did not receive the request or if the connection to the server was lost.
// # LDAP_NO_MEMORY if memory cannot be allocated.
// # LDAP_LOCAL_ERROR if an error occurred when receiving the results from the server.
// # LDAP_DECODING_ERROR if an error occurred when decoding the BER-encoded results from the server.
// # LDAP_NOT_SUPPORTED if controls are included in your request (for example, as a session preference) and your LDAP client does not specify that it is using the LDAPv3 protocol.

		
	GB_CONSTANT("Succes", "i", LDAP_SUCCESS),
	GB_CONSTANT("ParamError", "i", LDAP_PARAM_ERROR),
	GB_CONSTANT("EncodingError", "i", LDAP_ENCODING_ERROR),
	GB_CONSTANT("ServerDown", "i", LDAP_SERVER_DOWN),
	GB_CONSTANT("NoMemeory", "i", LDAP_NO_MEMORY),
	GB_CONSTANT("LocalError", "i", LDAP_LOCAL_ERROR),
	GB_CONSTANT("DecodingError", "i", LDAP_DECODING_ERROR),
	GB_CONSTANT("NotSupported", "i", LDAP_NOT_SUPPORTED),
	
	GB_CONSTANT("ScopeSubTree","i",LDAP_SCOPE_SUBTREE),
	GB_CONSTANT("Port","i",LDAP_PORT),
	
	GB_METHOD("Init","b",CLDAP_init,"(host)s(port)i"),
	GB_METHOD("Bind","i",CLDAP_bind,"(dn)s(pw)s"),
	GB_METHOD("UnBind","i",CLDAP_unbind,NULL),
	
	GB_METHOD("Search","Object[]",CLDAP_search,"(BaseDn)s(Scope)i(Filter)s[(SortBy)s]"),

	GB_END_DECLARE
};
/***************************************************************************

  LDAPMod

***************************************************************************/

#undef THIS
#define THIS ((CLDAPMOD *)_object)

BEGIN_METHOD_VOID(CLDAPMOD_new)
 
END_METHOD

BEGIN_METHOD_VOID(CLDAPMOD_free)

END_METHOD

BEGIN_PROPERTY(CLDAPMOD_op)


END_PROPERTY

BEGIN_PROPERTY(CLDAPMOD_type)

	if (READ_PROPERTY)
	{
		if (!THIS->mod)
			GB.ReturnNewString(NULL,0); 
		else
			GB.ReturnNewString((THIS->mod)->mod_type,0);
		
		return ;
	}
	
	GB.StoreString(PROP(GB_STRING), &THIS->mod->mod_type);

END_PROPERTY
BEGIN_PROPERTY(CLDAPMOD_values)

END_PROPERTY


GB_DESC CLDAPModDesc[] = 
{
	GB_DECLARE("LdapMod",sizeof(CLDAPMOD)),
    
	GB_METHOD("_new",NULL,CLDAPMOD_new,NULL),
	GB_METHOD("_free",NULL,CLDAPMOD_free,NULL),
	
	GB_PROPERTY("Op","i",CLDAPMOD_op),
	GB_PROPERTY("Type","s",CLDAPMOD_type),
	GB_PROPERTY("Values","String[]",CLDAPMOD_values),

	GB_END_DECLARE
};
/***************************************************************************

  LDAPEntry

***************************************************************************/

#undef THIS
#define THIS ((CLDAPSEARCHENTRY *)_object)

BEGIN_METHOD_VOID(CLDAPSEARCHENTRY_new)
 
END_METHOD

BEGIN_METHOD_VOID(CLDAPSEARCHENTRY_free)

END_METHOD
BEGIN_PROPERTY(CLDAPSEARCHENTRY_dn)
	
	if (READ_PROPERTY){
		if (!THIS->dn)
			GB.ReturnNewString(NULL,0); 
		else
			GB.ReturnNewString(THIS->dn,0);
			
		return ;
	}
	
	GB.StoreString(PROP(GB_STRING), &THIS->dn);
	
END_METHOD

BEGIN_METHOD_VOID(CLDAPSEARCHENTRY_getentries)
 	 GB.ReturnObject(THIS->attributes);
 END_METHOD

GB_DESC CLDAPSearchEntryDesc[] = 
{
	GB_DECLARE("LdapSearchEntry",sizeof(CLDAPSEARCHENTRY)),
    
	GB_PROPERTY("Dn","String",CLDAPSEARCHENTRY_dn),
	
	GB_METHOD("_new",NULL,CLDAPSEARCHENTRY_new,NULL),
	GB_METHOD("_free",NULL,CLDAPSEARCHENTRY_free,NULL),
	
	GB_METHOD("GetEntries","Collection",CLDAPSEARCHENTRY_getentries,NULL),

	GB_END_DECLARE
};

/***************************************************************************

  LDAPMessage

***************************************************************************/

#undef THIS
#define THIS ((CLDAPMESSAGE *)_object)

BEGIN_METHOD_VOID(CLDAPMESSAGE_new)
 
END_METHOD

BEGIN_METHOD_VOID(CLDAPMESSAGE_free)

END_METHOD

GB_DESC CLDAPMessageDesc[] = 
{
	GB_DECLARE("LdapMessage",sizeof(CLDAPMESSAGE)),
    
	GB_METHOD("_new",NULL,CLDAPMESSAGE_new,NULL),
	GB_METHOD("_free",NULL,CLDAPMESSAGE_free,NULL),

	GB_END_DECLARE
};

