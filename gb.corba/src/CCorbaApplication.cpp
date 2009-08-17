/***************************************************************************

  CCorbaApplication.cpp

  (c) 2005 Carlo Sorda <gambas@users.sourceforge.net><csorda@libero.it>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#define __CCORBA_APPLICATION_CPP


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "main.h"
#include "gambas.h"
#include "gb_common.h"
#include "CCorbaApplication.h"

#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

//#define CORBA_DEBUG 0
 
//CORBA::Object_var corbaobj;

	
typedef
  struct {
    char *dcop;
    GB_TYPE type;
    }
  TYPE_CONV;
  
void CORBA_init(void)
{
}
void CORBA_exit(void)
{
  
}

// static void hello(CORBA::Object_ptr obj)
// {
//   
// CORBA::String_var arg = (const char*) "Hello!";
//   CORBA::Request_var req = obj->_request("echoString");
//   req->add_in_arg() <<= arg;
//   req->set_return_type(CORBA::_tc_string);
// 
//   req->invoke();
// 
//   if( req->env()->exception() ) {
//     cout << "echo_diiclt: An exception was thrown!" << endl;
//     return;
//   }
// 
//   const char* ret;
//   req->return_value() >>= ret;
// 
//   cerr << "I said, \"" << (char*)arg << "\"." << endl
//        << "The Echo object replied, \"" << ret <<"\"." << endl;
// }
// static void hello_deferred(CORBA::Object_ptr obj)
// {
//   CORBA::String_var arg = (const char*) "Hello!";
//   CORBA::Request_var req = obj->_request("echoString");
//   req->add_in_arg() <<= arg;
//   req->set_return_type(CORBA::_tc_string);
// 
//   req->send_deferred();
//   cerr << "Sending deferred request: ";
//   while( !req->poll_response() )
//     cerr << '#';
//   cerr << endl << "Response received." << endl;
// 
//   if( req->env()->exception() ) {
//     cout << "echo_diiclt: An exception was thrown!" << endl;
//     return;
//   }
// 
//   const char* ret;
//   req->return_value() >>= ret;
// 
//   cerr << "I said, \"" << (char*)arg << "\"." << endl
//        << "The Echo object replied, \"" << ret <<"\"." << endl;
// }
static CORBA::Object_ptr
getObjectReference(CORBA::ORB_ptr orb, CCORBA_COSNAMING *CosNaming_)
{
  	//corbalocAddress = "corbaloc::iiop:172.22.201.24:10435/NameService";
	CosNaming::NamingContext_var rootContext;
	#ifdef CORBA_DEBUG		
  		cout << "prima di try" << endl;
	#endif
  	try {
    	// Obtain a reference to the root context of the Name service:
    	CORBA::Object_var obj;
    	obj = orb->resolve_initial_references("NameService");
	
		//cout << "dentro try " << corbalocAddress << endl;
    	// Narrow the reference returned.
    	rootContext = CosNaming::NamingContext::_narrow(obj);
		#ifdef CORBA_DEBUG		
			cout << "rootContext = CosNaming::NamingContext::_narrow(obj);" << endl;
		#endif

    	if( CORBA::is_nil(rootContext) ) {
      			cerr << "Failed to narrow the root naming context." << endl;
      	return CORBA::Object::_nil();
    	}
  	}
  catch(CORBA::ORB::InvalidName& ex) {
    // This should not happen!
	GB.Error("Service required is invalid [does not exist].");
    cerr << "Service required is invalid [does not exist]." << endl;
    return CORBA::Object::_nil();
  }
  catch(...) {
    // This should not happen!
	GB.Error("Errore generale");
    cerr << "Errore generale." << endl;
    return CORBA::Object::_nil();
  }
	

	#ifdef CORBA_DEBUG		
		cout << "dopo try"<< endl;
	#endif
  	// Create a name object, containing the name test/context:
 	CosNaming::Name name;
  	name.length(CosNaming_->name.length());

  	for (unsigned int i =0;i<CosNaming_->name.length();i++){
		name[i].id   = CosNaming_->name[i].id;       // string copied
		#ifdef CORBA_DEBUG		
			cout << "id=" << name[i].id << endl; 
		#endif
  		if ( CosNaming_->name[i].kind != 0 ){
			name[i].kind   = CosNaming_->name[i].kind;       // string copied
			#ifdef CORBA_DEBUG		
				cout << "kind=" << name[i].kind << endl; 
			#endif
			}

	  }
  
  	//~ name[0].id   = (const char*) CosNaming_->id;       // string copied
	//~ if ( CosNaming_->kind != 0 )
		//~ name[0].kind   = (const char*) CosNaming_->kind;       // string copied

	#ifdef CORBA_DEBUG		
			cout << "Dopo assegnazione name"<< endl; 
	#endif

  //name[0].kind = (const char*) ""; // string copied
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)


// Create a name object, containing the name test/context:
//   CosNaming::Name name;
//   name.length(2);
// 
//   name[0].id   = (const char*) "test";       // string copied
//   name[0].kind = (const char*) "my_context"; // string copied
//   name[1].id   = (const char*) "Echo";
//   name[1].kind = (const char*) "Object";

  	try {
    	// Resolve the name to an object reference.
		#ifdef CORBA_DEBUG		
			cout << "Prima di resolve()"<< endl; 
		#endif
		return rootContext->resolve(name);
		
  	}
  	catch(CosNaming::NamingContext::NotFound& ex) {
    	// This exception is thrown if any of the components of the
    	// path [contexts or the object] aren't found:
		GB.Error("Context not found.");
    	cerr << "Context not found." << endl;
  	}
  	catch(CORBA::COMM_FAILURE& ex) {
		GB.Error("Caught system exception COMM_FAILURE -- unable to contact the naming service.");
    	cerr << "Caught system exception COMM_FAILURE -- unable to contact the " << "naming service." << endl;
  	}
  	catch(CORBA::SystemException&) {
		GB.Error("Caught a CORBA::SystemException while using the naming service.");
    	cerr << "Caught a CORBA::SystemException while using the naming service." << endl;
  	}

  return CORBA::Object::_nil();
}
//~ static void getdress(CORBA::Object_ptr obj)
//~ {
  	//~ cout << "Sto eseguendo getddress()" << endl;
	//~ CORBA::String_var arg = (const char*) "34343434";
  	//~ CORBA::String_var arg1 = (const char*) "3434";

  	//~ CORBA::Request_var req = obj->_request("getddress");
  	//~ req->add_in_arg() <<= arg;
	//~ req->add_in_arg() <<= arg1;
  	//~ req->set_return_type(CORBA::_tc_string);

  	//~ req->invoke();

  	//~ if( req->env()->exception() ) {
    	//~ cout << "echo_diiclt: An exception was thrown!" << endl;
    	//~ return;
  	//~ }

  //~ const char* ret;
  //~ req->return_value() >>= ret;

  //~ cerr << "I said, \"" << (char*)arg << "\"." << "\"" << (char*)arg1 << "\"." <<endl
       //~ << "The Echo object replied, \"" << ret <<"\"." << endl;
//~ }
/***************************************************************************

 CORBA

***************************************************************************/
//callIdlMethod(retType, GB.GetUnknown(), ARG(param[0]), GB.NParam()))

static bool callIdlMethod(CORBA::Object_var corbaobj, const char *name, GB_VALUE *args, int nparam)
{
	//hello(corbaobj);	
	GB_VALUE *arg;
	int i,n,retType;
	CORBA::Request_var req;
	//DynamicAny::DynAnyFactory_var fac;

	n = nparam;
	
	#ifdef CORBA_DEBUG
		cout << name << " ( ";
	#endif
	
	//arg = &args[0];	
	arg = &args[0];
	retType = ((GB_INTEGER *)arg)->value;

// 	1 Type 4 AS Integer
// 	2 Type 7 AS Float
// 	3 Type 8 AS Date
// 	4 Type 3 AS Short
// 	5 Type 2 AS Byte
// 	6 Type 5 AS Long
// 	7 Type 16 AS Object
// 	8 Type 9-10 AS String
// 	9 Type 6 AS Single
// 	10 Type 15 AS Variant
	req = corbaobj->_request(name);

   for (i = 1; i < n; i++)
  	{
    	arg = &args[i];	
		
		#ifdef CORBA_DEBUG
			cout << "Count=" << i << " " << " Type = " << arg->type << endl;
		#endif
		
		//string -> char*
		if (arg->type == 10){
			CORBA::String_var argument = (const char*)GB.ToZeroString((GB_STRING*)arg);
			//CORBA::String_var argument = (const char*)"2323";
			
			req->add_in_arg() <<= argument;
			#ifdef CORBA_DEBUG
			if (i >  1)
				cout << ",";

		   cout << "\"" << argument << "\"";
			#endif
		}
		//integer -> short(int)
		//short -> short
		else if (arg->type == 4 || arg->type == 3){
			CORBA::Short value = ((GB_INTEGER *)arg)->value;
			//req = corbaobj->_request(name);
			req->add_in_arg() <<= value;
			#ifdef CORBA_DEBUG
			if (i >  1)
				cout << ",";
			
			cout << value;
			#endif
			}
		//float -> float
		else if (arg->type == 7){
			CORBA::Double value = ((GB_FLOAT *)arg)->value;
			//req = corbaobj->_request(name);
			req->add_in_arg() <<= value;
			#ifdef CORBA_DEBUG
			if (i ==  1)
				cout << value;
			else
				cout << "," << value;
			#endif
			}
					//float -> float
		else if (arg->type == 6){
			CORBA::Float value = ((GB_FLOAT *)arg)->value;
			//req = corbaobj->_request(name);
			req->add_in_arg() <<= value;
			#ifdef CORBA_DEBUG
			if (i ==  1)
				cout << value;
			else
				cout << "," << value;
			#endif
			}
		//long -> long
		else if (arg->type == 5){
			CORBA::Long value = ((GB_LONG *)arg)->value;
			//req = corbaobj->_request(name);
			req->add_in_arg() <<= value;
			#ifdef CORBA_DEBUG
			if (i ==  1)
				cout << value;
			else
				cout << "," << value;
			#endif
			}
// 		//Boolean
// 		else if (arg->type == 1){
// 			CORBA::Long value = ((GB_BOOLEAN *)arg)->value;
// 			req->add_in_arg() <<= value;
// 			#ifdef CORBA_DEBUG
// 			if (i >  1)
// 				cout << ",";
// 			
// 			cout << value;
// 			#endif
// 			}


	}
	#ifdef CORBA_DEBUG		
		cout << " ) " << endl;
	#endif

	if (retType == 0)
		req->set_return_type(CORBA::_tc_void);	
	else if (retType == 10)
		req->set_return_type(CORBA::_tc_string);	
	else if (retType == 4 || retType == 3)
		req->set_return_type (CORBA::_tc_short);
	else if (retType == 7)	
		req->set_return_type (CORBA::_tc_float);
	else if (retType == 8)	
		req->set_return_type (CORBA::_tc_double);
	else if (retType == 5)	
		req->set_return_type (CORBA::_tc_long);
	else if (retType == 16)	
		req->set_return_type (CORBA::_tc_Object);
	else if (retType == 20)	
		req->set_return_type (CORBA::_tc_ushort);
	else if (retType == 30){	
		req->set_return_type (CORBA::_tc_any);
		cout << "rettype = 30" << endl;
		}
// 	else if (retType == 1)	
// 		req->set_return_type (CORBA::_tc_boolean);

  req->invoke();

	if( req->env()->exception() ) {
   		cout << "An exception was thrown!" << endl;
		GB.Error("An exception was thrown!");
		return false;
  	}
	#ifdef CORBA_DEBUG		
	cout << "Command was executed!"<<endl;
	#endif

	if (retType == 0){
		return true;
	}
	//string
	if (retType == 10){
		const char* ret;
  		req->return_value() >>= ret;
		GB.ReturnNewZeroString(ret);
	}
	//int
	else if (retType == 4 || retType == 3){
		CORBA::Short ret = 0;
		req->return_value() >>= ret;
		GB.ReturnInteger(ret);
	}
	//float
	else if (retType == 20 ){
		CORBA::UShort ret = 0;
		req->return_value() >>= ret;
		GB.ReturnInteger(ret);
	}

	//float
	else if (retType == 7 ){
		CORBA::Float ret = 0;
		req->return_value() >>= ret;
		GB.ReturnFloat(ret);
	}
	else if (retType == 8 ){
		CORBA::Double ret = 0;
		req->return_value() >>= ret;
		GB.ReturnFloat(ret);
	}

	else if (retType == 5 ){
		CORBA::Long ret = 0;
		req->return_value() >>= ret;
		GB.ReturnLong(ret);
	}
	else if (retType == 16){
		CCORBA_OBJECT *_object = 0;	
		GB.New(POINTER(&_object), GB.FindClass("CORBAObject"), NULL, NULL);
  		//_object->ref = new DCOPRef(dcopref);

		//req->set_return_type (CORBA::_tc_Object);
		// Extract the returned object reference from the request.
      req->return_value () >>= CORBA::Any::to_object (_object->obj_var_.out ());
		GB.ReturnObject(_object);
 	}
	else if (retType == 30){
		#ifdef CORBA_DEBUG		
			cout << "retType=30" << endl;
		#endif
		GB_ARRAY array;
    	//const char *data;
		CORBA::Any temp;

    	//req->set_return_type (CORBA::_tc_any);
		req->return_value () >>= temp;
		
		CORBA::StringSeq* elem_ptr = new  CORBA::StringSeq();
		
		temp >>= elem_ptr;
		
		
		n =elem_ptr->length();
		cout << "n:" << n << endl;

    	GB.Array.New(&array, GB_T_STRING, n);
    	for (i = 0; i < n; i++)
    	{
      		//data = *elem_ptr[i];
      		//GB.NewString((char **)GB.Array.Get(array, i), data, 0);
    	}

    	GB.ReturnObject(array);
		
		
		
		//~ CCORBA_OBJECT *_object = 0;	
		//~ GB.New((void **)&_object, GB.FindClass("CORBAObject"), NULL, NULL);
  		//~ //_object->ref = new DCOPRef(dcopref);

		//~ req->set_return_type (CORBA::_tc_Object);
		//~ // Extract the returned object reference from the request.
      //~ req->return_value () >>= CORBA::Any::to_object (_object->obj_var_.out ());
		//~ GB.ReturnObject(_object);
 	}


/*	//Boolean
	else if (retType == 1 ){
		CORBA::Boolean ret = false;
		req->return_value() >>= ret;
		GB.ReturnBoolean(ret);
	}*/
	return false;
}
#undef THIS
#define THIS ((CCORBA_COSNAMING *)_object)
BEGIN_METHOD_VOID(CCORBA_COSNAMING_new)
 	THIS->kind = 0;
	THIS->id = 0;

	//~ for (int i =0;i<THIS->name.length();i++){
			//~ name[i].id   = CosNaming_->name[i].id;       // string copied
			//~ if ( CosNaming_->name[i].kind != 0 )
				//~ name[i].kind   = CosNaming_->name[i].kind;       // string copied
	
		  //~ }
	  
END_METHOD

BEGIN_METHOD_VOID(CCORBA_COSNAMING_free)
	
END_METHOD
BEGIN_METHOD_VOID(CCORBA_COSNAMING_length)
	
	if (READ_PROPERTY)
    	GB.ReturnInteger(THIS->name.length());
  	else
    	THIS->name.length((VPROP(GB_INTEGER)));
END_METHOD
BEGIN_PROPERTY(CCORBA_COSNAMING_id)
	char* id_ = 0;
	
	if (READ_PROPERTY){
		if (!THIS->name[THIS->index].id)
			GB.ReturnNewString(NULL,0); 
		else
			GB.ReturnNewString(THIS->name[THIS->index].id,0);
			
		return ;
	}
	GB.StoreString(PROP(GB_STRING), &id_);
	
	THIS->name[THIS->index].id = strdup(id_);
	#ifdef CORBA_DEBUG		
		cout << "id = " << id_<< endl; 
	#endif
	GB.FreeString(&id_);
	
END_METHOD
BEGIN_PROPERTY(CCORBA_COSNAMING_kind)
	char* kind_=0;
	
	if (READ_PROPERTY){
		if (!THIS->name[THIS->index].kind)
			GB.ReturnNewString(NULL,0); 
		else
			GB.ReturnNewString(THIS->name[THIS->index].kind,0);
			
		return ;
	}
	
	GB.StoreString(PROP(GB_STRING), &kind_);
	THIS->name[THIS->index].kind = strdup(kind_);
	#ifdef CORBA_DEBUG		
		cout << "kind = " << kind_<< endl; 
	#endif
	GB.FreeString(&kind_);
	
	
END_METHOD

BEGIN_METHOD(CCORBA_COSNAMING_get, GB_INTEGER index)

  	long index = VARG(index);

	THIS->index = index;
  	RETURN_SELF();

END_METHOD

GB_DESC CCORBACosNamingNameDesc[] =
{
  	GB_DECLARE(".CosNamingName",0), GB_VIRTUAL_CLASS(),

  	GB_PROPERTY("id",   "s", CCORBA_COSNAMING_id),
  	GB_PROPERTY("kind", "s", CCORBA_COSNAMING_kind),

	GB_END_DECLARE
};
GB_DESC CCORBACosNamingNamesDesc[] =
{
	  GB_DECLARE("CORBACosNamingName", sizeof(CCORBA_COSNAMING)),
	
	  GB_METHOD("_free", NULL, CCORBA_COSNAMING_free, NULL),
	
	  //~ GB_PROPERTY("id", "s", CCORBA_COSNAMING_id),
	  //~ GB_PROPERTY("kind", "s", CCORBA_COSNAMING_kind),
	
		GB_METHOD("_get", ".CosNamingName", CCORBA_COSNAMING_get, "(Index)i"),
		GB_PROPERTY("length", "i", CCORBA_COSNAMING_length),
	//GB_METHOD("_put", NULL, CCORBA_COSNAMING_put, "(id)s(kind)s"),
	//GB_METHOD("length", NULL, CCORBA_COSNAMING_length, "(length)i"),

  GB_END_DECLARE
};

#undef THIS
#define THIS ((CCORBA_APPLICATION *)_object)

/*!
	CORBAApplication
	E' possibile creare un oggetto CORBAClient attraverso lo IOR del server CORBA di 
	cui si vogliono richiedere i servizi.
	E' Possibile creare un oggetto CORBAClient attraverso un URI dell'oggetto remoto o del NameServer
	
	GAMBAS2 Sample
	Statis Sub main()
		Dim CorbaApp as CORBAApplication
		Dim IOR as string
		Dim URI as string

		IOR = "IOR:00945405945045945094509...."
		CorbaApp = new CORBAApplication(IOR)
		
		'or you can crete a new object by URI like this
		URI = "corbaloc:iiop:localhost:10435/Nameservice"
		CorbaApp = new CORBAApplication(IOR)
		
	end sub
*/
BEGIN_METHOD_VOID(CCORBA_APPLICATION_new)
END_METHOD

BEGIN_METHOD(CCORBA_APPLICATION_InitIOR,GB_STRING IOR)
	
	int 	argc 		= 1;
	char 	**argv	= 0;
	char* sIOR_;
	CCORBA_OBJECT *_obj = 0;	
	
	sIOR_= GB.ToZeroString(ARG(IOR));
	if (strlen(sIOR_) == 0){
		GB.Error("IOR must be not zero lenght!");
		return;
	}	

	//strcpy(THIS->IOR,sIOR_); 
	argv = (char**)malloc(sizeof(char*)*(1));	
	argv[0] = strdup("omniORB4");

 	try {
		THIS->orb = CORBA::ORB_init(argc, argv, "omniORB4");
		
		if( CORBA::is_nil(THIS->orb) ) 
			GB.Error("ORB not initialized!");

			
/*      THIS->obj_var_ = THIS->orb->string_to_object(sIOR_);
		if( CORBA::is_nil(THIS->obj_var_) ) 
			GB.Error("Remote CORBA OBJECT not initialized!");*/
		
		GB.New(POINTER(&_obj), GB.FindClass("CORBAObject"), NULL, NULL);
		_obj->obj_var_ = THIS->orb->string_to_object(sIOR_);

		free(argv[0]);
		free(argv);	

		GB.ReturnObject(_obj);

  }
  catch(CORBA::COMM_FAILURE& ex) {
	GB.Error("Caught system exception COMM_FAILURE -- unable to contact the object.");
   cerr << "Caught system exception COMM_FAILURE -- unable to contact the object." << endl;
  }
  catch(CORBA::SystemException&) {
	GB.Error("Caught a CORBA::SystemException.");
	cerr << "Caught a CORBA::SystemException." << endl;
  }
  catch(CORBA::Exception&) {
	GB.Error("Caught CORBA::Exception.");
    cerr << "Caught CORBA::Exception." << endl;
  }
  catch(omniORB::fatalException& fe) {
	GB.Error("Caught omniORB::fatalException:");     
	cerr << "Caught omniORB::fatalException:" << endl;
/*     cerr << "  file: " << fe.file() << endl;
     cerr << "  line: " << fe.line() << endl;
     cerr << "  mesg: " << fe.errmsg() << endl;*/
  }
  catch(...) {
	GB.Error("Caught unknown exception.");
    cerr << "Caught unknown exception." << endl;
  }

	
	
	//GB.ReturnBoolean(true);
END_METHOD
BEGIN_METHOD(CCORBA_APPLICATION_InitURI,GB_OBJECT CosNamingName;GB_STRING NameServiceURI)
	
	int 	argc 		= 3;
	char 	**argv	= 0;

	//char* URI_;
	//char* CorbaServerName_;
	CCORBA_OBJECT *_obj = 0;
	CCORBA_COSNAMING *CosNaming_ = 0;	
	
	THIS->URI = GB.ToZeroString(ARG(NameServiceURI));

    argv = (char**)malloc(sizeof(char*)*(3));	
	argv[0] = strdup("omniORB4");
	argv[1] = strdup("-ORBInitRef");
 	//argv[2] = strdup("NameService=corbaloc:iiop:stargate:10435/NameService");
	argv[2] = THIS->URI;	


	CosNaming_ = (CCORBA_COSNAMING*) VARG(CosNamingName);

	if (strlen(THIS->URI) == 0){
		GB.Error("NameServiceURI must be not zero lenght!");
		return;
	}	
	
// cout << "riga 3" << endl;
// 	strcpy(THIS->CorbaServerName,CorbaServerName_); 
// cout << "riga 4" << endl;
// 	strcpy(THIS->URI,URI_); 
// cout << "riga 5" << endl;

 	try {
		THIS->orb = CORBA::ORB_init(argc, argv);
		if( CORBA::is_nil(THIS->orb) ) 
			GB.Error("ORB not initialized!");

		GB.New(POINTER(&_obj), GB.FindClass("CORBAObject"), NULL, NULL);
		_obj->obj_var_ = getObjectReference(THIS->orb,CosNaming_);
		#ifdef CORBA_DEBUG		
		cout << "Dopo il getObject..."<< endl;
		#endif

// 		free(argv[2]);
// 		free(argv[1]);
// 		free(argv[0]);
// 		free(argv);	

		GB.ReturnObject(_obj);
}
  catch(CORBA::COMM_FAILURE& ex) {
	GB.Error("Caught system exception COMM_FAILURE -- unable to contact the object.");
   cerr << "Caught system exception COMM_FAILURE -- unable to contact the object." << endl;
  }
  catch(CORBA::SystemException&) {
	GB.Error("Caught a CORBA::SystemException.");
	cerr << "Caught a CORBA::SystemException." << endl;
  }
  catch(CORBA::Exception&) {
	GB.Error("Caught CORBA::Exception.");
    cerr << "Caught CORBA::Exception." << endl;
  }
  catch(omniORB::fatalException& fe) {
	GB.Error("Caught omniORB::fatalException:");     
	cerr << "Caught omniORB::fatalException:" << endl;
  }
  catch(...) {
	GB.Error("Caught unknown exception.");
    cerr << "Caught unknown exception." << endl;
  }

/*cout << "prima della liberazione della memoria" << endl;

	free(argv[2]);
	free(argv[1]);
	free(argv[0]);
	free(argv);	
cout << "dopo della liberazione della memoria" << endl;*/
END_METHOD

BEGIN_METHOD_VOID(CCORBA_APPLICATION_free)
	//THIS->orb must not nil
	THIS->orb->destroy();
END_METHOD
BEGIN_METHOD_VOID(CCORBA_APPLICATION_exit)
	
END_METHOD
BEGIN_METHOD(CCORBA_APPLICATION_unknown, GB_VALUE param[0])

	//GB_VALUE *arg;
//	//int i;
	int n;
	//int retType;
	GB_VALUE *args;

	n = GB.NParam();
	args = ARG(param[0]);

//    for (i = 1; i < n; i++)
//   	{
//     	arg = &args[i];	
// 		printf("i= %d  Type= %d\n",i,arg->type);	
// 	}
// 	return ;

  	if (GB.IsProperty())
  	{
	
/*		if (call_method(THIS, NULL, GB.GetUnknown(), NULL, 0, false))
			get_object(THIS, GB.GetUnknown());
 */ 	
		//Return type
// 		arg = &args[0];
// 		retType = ((GB_INTEGER *)arg)->value;
// 		
// 		if (callIdl(retType, GB.GetUnknown(), ARG(param[0]), GB.NParam()))
// 			cout << "Tutto bene" << endl;
// 		else
// 			cout << "Errore" << endl;
// 		
		//call_method(THIS, THIS->object, GB.GetUnknown(), ARG(param[0]), GB.NParam(), true);
		//GB.FreeString(&THIS->object);

	}
  	else
  	{
	
		//Return type
		if (callIdlMethod(THIS->obj_var_,GB.GetUnknown(), ARG(param[0]), GB.NParam())){
		#ifdef CORBA_DEBUG			
		cout << "Tutto bene" << endl;
			#endif
		}
		else{
			#ifdef CORBA_DEBUG		
			cout << "Errore" << endl;
			#endif
		}
		
		//call_method(THIS, THIS->object, GB.GetUnknown(), ARG(param[0]), GB.NParam(), true);
		//GB.FreeString(&THIS->object);
  }

END_METHOD


	
GB_DESC CCORBAApplicationDesc[] = 
{
	GB_DECLARE("CORBAApplication",sizeof(CCORBA_APPLICATION)),
	GB_METHOD("_new",NULL,CCORBA_APPLICATION_new,NULL),	
  	GB_METHOD("_free", NULL, CCORBA_APPLICATION_free, NULL),

	GB_CONSTANT("CorbaStringSequence", "i", 30),
	GB_CONSTANT("CorbaObjectSequence", "i", 31),
	GB_CONSTANT("CorbaObject", "i", 16),
	GB_CONSTANT("CorbaFloat", "i", 7),
	GB_CONSTANT("CorbaDouble", "i", 8),
	GB_CONSTANT("CorbaString", "i", 10),
	GB_CONSTANT("CorbaShort", "i", 3),
	GB_CONSTANT("CorbaUShort", "i", 20),

	GB_CONSTANT("CorbaInteger", "i", 4),
	GB_CONSTANT("CorbaLong", "i", 5),
	GB_CONSTANT("CorbaVoid", "i", 0),
	
	GB_STATIC_METHOD("_exit", NULL, CCORBA_APPLICATION_exit, NULL),
  	GB_METHOD("_unknown", "v", CCORBA_APPLICATION_unknown, "."),

	//GB_METHOD("InitIOR", "b", CCORBA_APPLICATION_InitIOR, "(IOR)s"),
	GB_METHOD("InitIOR", "CORBAObject", CCORBA_APPLICATION_InitIOR, "(IOR)s"),
	GB_METHOD("InitURI", "CORBAObject", CCORBA_APPLICATION_InitURI, "(CosNamingName)CORBACosNamingName;(NameServiceURI)s"),
	
	GB_END_DECLARE
};

#undef THIS
#define THIS ((CCORBA_OBJECT *)_object)

BEGIN_METHOD_VOID(CCORBA_OBJECT_free)

END_METHOD

BEGIN_PROPERTY(CCORBA_OBJECT_IOR)
	GB.ReturnNewZeroString(THIS->IOR);
END_PROPERTY
BEGIN_METHOD(CCORBA_OBJECT_unknown, GB_VALUE param[0])

  callIdlMethod(THIS->obj_var_, GB.GetUnknown(), ARG(param[0]), GB.NParam());

END_METHOD

GB_DESC CCORBAObjectDesc[] =
{
  GB_DECLARE("CORBAObject", sizeof(CCORBA_OBJECT)), GB_NOT_CREATABLE(),

  GB_METHOD("_free", NULL, CCORBA_OBJECT_free, NULL),
  GB_METHOD("_unknown", "v", CCORBA_OBJECT_unknown, "."),

  GB_PROPERTY_READ("IOR", "s", CCORBA_OBJECT_IOR),
  GB_END_DECLARE
};
