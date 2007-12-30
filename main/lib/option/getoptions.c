

#define __GETOPTIONS_C

#include "gb_common.h"
#include<string.h>
#include<stdlib.h>
#include "getoptions.h"
#include "main.h"
#include <unistd.h>






BEGIN_METHOD_VOID (getopt_calc)
{
	char **tmp,c,charec[2]={0};
	int i;
	
	THIS->index=0;
	if(THIS->opt_arg!=NULL)
	{
		
		for (i = 0; i < GB.Count(THIS->opt_arg); i++) 
		{
			GB.FreeString(&(THIS->opt_arg[i]));
		}
		GB.FreeArray((void *) &(THIS->opt_arg)); 
	}
	
	if(THIS->opt_found!=NULL)
	{
		for (i = 0; i < GB.Count(THIS->opt_found); i++) 
		{
			GB.FreeString(&(THIS->opt_found[i]));
		}
		GB.FreeArray((void *) &(THIS->opt_found)); 
	}
	opterr=0;
	optind=0;
	GB.NewArray((void *) &(THIS->opt_found), sizeof(*(THIS->opt_found)), 0);
	GB.NewArray((void *) &(THIS->opt_arg), sizeof(*(THIS->opt_arg)), 0);
	GB.NewArray((void *) &(THIS->invalid), sizeof(*(THIS->invalid)), 0);

	
	
	while(1)
	{
		c=getopt(THIS->arg_count,THIS->argv,THIS->options);
		if(c==-1)
		{
			break;
		}
		
		
		charec[0]=c;
		tmp=(char **) GB.Add((void *) &(THIS->opt_found));
		GB.NewString(tmp,charec,0);
		
		charec[0]=optopt;
		tmp=(char **) GB.Add((void *) &(THIS->invalid));
		if(optopt!=0)
		{
			GB.NewString(tmp,charec,0);
		}
		else
		{
			GB.NewString(tmp,"",0);
		}

		tmp=(char **) GB.Add((void *) &(THIS->opt_arg));
		if(optarg!=NULL)
		{
			GB.NewString(tmp,optarg,0);
		}
		else
		{
			GB.NewString(tmp,"",0);
		}
	}
	if(THIS->rest!=NULL)
	{
		GB.Unref((void *)&THIS->rest);
	}
	
	GB.Array.New((void *) &(THIS->rest) , GB_T_STRING, THIS->arg_count-optind);
	GB.Ref(THIS->rest);
	for(i=optind;i<THIS->arg_count;i++)
	{
		tmp=(char **)GB.Array.Get(THIS->rest,i-optind);
		GB.NewString(tmp,THIS->argv[i],0);
	}
}
END_METHOD

BEGIN_METHOD_VOID ( COPTIONS_next )
{
	char *str;

	if(THIS->index >= GB.Count(THIS->opt_found) )
	{
		GB.ReturnNewZeroString ("");
		return;
	}
	str=THIS->opt_found [ THIS->index ];
	GB.ReturnNewZeroString (str);
	THIS->index++;
}
END_METHOD

BEGIN_METHOD_VOID ( COPTIONS_invalid )
{
	char *str;
	
	if(THIS->index > GB.Count(THIS->invalid) || THIS->index<=0)
	{
		GB.ReturnNewZeroString ("");
		return;
	}
	printf("%d\n",THIS->index);
	str=THIS->invalid [ THIS->index-1 ];
	GB.ReturnNewZeroString (str);
}
END_METHOD

BEGIN_METHOD_VOID (COPTIONS_opt_arg)
{
	char *str;
	
	if(THIS->index > GB.Count(THIS->opt_arg) || THIS->index<=0)
	{
		GB.ReturnNewZeroString ("");
		return;
	}
	
	str=THIS->opt_arg [ THIS->index - 1 ];
	GB.ReturnNewZeroString (str);
}
END_METHOD

BEGIN_METHOD_VOID(COPTIONS_rest)
{
	GB.ReturnObject(THIS->rest);
}
END_METHOD

BEGIN_METHOD ( COPTIONS_get, GB_STRING option )
{
	int i;
	char *str=NULL;
	for ( i=0; i<GB.Count(THIS->opt_found); i++ )
	{
		if( THIS->opt_found[i][0]==STRING(option)[0] )
		{
			if(THIS->opt_arg[i]!=NULL && STRING(option)[0]!='?')
			{
				str=THIS->opt_arg[i];
			}
			else
			{
				GB.ReturnInteger(TRUE);
				return;
			}
		}
	}
	if(str!=NULL)
	{
		GB.ReturnNewZeroString(str);
		return;
	}
	

	/* should i chUck this out?
	for ( i=0; i<GB.Count(THIS->invalid); i++ )
	{
		if( THIS->invalid[i]!=NULL )
		{
			if( THIS->invalid[i][0]==STRING(option)[0] )
			{
				GB.ReturnInteger(TRUE);
				return;
			}
		}
	}
	*/
	
	GB.ReturnInteger(FALSE);
	return;
}
END_METHOD

BEGIN_METHOD ( COPTIONS_getarg ,GB_STRING option)
{
	int i;
	char **tmp;
	if(THIS->return_temp!=NULL)
	{
		GB.Unref(&THIS->return_temp);
	}
	
	GB.Array.New(&THIS->return_temp,GB_T_STRING,0);
	for(i=0; i<GB.Count( THIS->opt_found ) ; i++ )
	{
		if( THIS->opt_found[i][0] == STRING(option)[0] )
		{
			if(THIS->opt_arg[i]!=NULL )
			{
				tmp=(char **)GB.Array.Add(THIS->return_temp);
				GB.NewString(tmp,THIS->opt_arg[i],0);
			}
		}
	}
	GB.Ref(THIS->return_temp);
	GB.ReturnObject(THIS->return_temp);
}
END_METHOD

BEGIN_METHOD_VOID ( COPTION_getallopt)
{
	int i;
	char **tmp;
	char charec[2]={0};
	if ( THIS->return_temp!=NULL )
	{
		GB.Unref(&THIS->return_temp);
	}
	
	GB.Array.New(&THIS->return_temp,GB_T_STRING,0);
	for(i=0; i<GB.Count( THIS->opt_found ) ; i++ )
	{
		if( THIS->opt_found[i][0] != '?' && THIS->opt_found[i][0] != ':')
		{
			if(THIS->opt_found[i]!=NULL )
			{
				tmp=(char **)GB.Array.Add(THIS->return_temp);
				charec[0]=THIS->opt_found[i][0];
				GB.NewString(tmp,charec,0);
			}
		}
	}
	GB.Ref(THIS->return_temp);
	GB.ReturnObject ( THIS->return_temp );
}
END_METHOD

BEGIN_PROPERTY(COPTIONS_cmdline)
{
	GB.ReturnObject(THIS->cmdline);
}
END_PROPERTY

BEGIN_PROPERTY(COPTIONS_options)
	GB.NewString(&THIS->options,PSTRING(),0);
END_PROPERTY
		
BEGIN_METHOD ( COPTIONS_new, GB_STRING options; GB_OBJECT array )
{
	char **tmp,*src;
	int i,narg;

	GB.NewString(&(THIS->options),STRING(options),0);

     GB.NewArray((void *) &(THIS->argv), sizeof(*(THIS->argv)), 0); // argv is where i keep track of what to free later
	GB.Array.New((void *) &(THIS->cmdline) ,GB_T_STRING,arg_count);
	GB.Ref(THIS->cmdline);
	for(i=0;i<arg_count;i++)
	{
		tmp=(char **)GB.Array.Get((void*)(THIS->cmdline),i);
		GB.NewString(tmp,cmd_arg[i],0);
	}

	if ( MISSING ( array ) )
	{
		for ( i=0; i<arg_count; i++ )
		{
			tmp=(char **)GB.Add((void *) &(THIS->argv));
			GB.NewString(tmp,cmd_arg[i],0);
		}
		THIS->arg_count=arg_count;
	}
	else
	{
		if(VARG(array)!=NULL)
		{
			narg = GB.Array.Count ( VARG ( array ) );
			for(i=0;i<narg;i++)
			{
				src=*(char **)GB.Array.Get( VARG ( array ),i);
				tmp=(char **) GB.Add((void *) &(THIS->argv));
				GB.NewString(tmp,src,0);
			}
			THIS->arg_count=narg;
		}
	}

	/* start getting the options and store it*/
	
	THIS->opt_arg=NULL;
	THIS->opt_found=NULL;
	THIS->return_temp=NULL;
	THIS->index=0;
	CALL_METHOD_VOID(getopt_calc);
	
	RETURN_SELF();
	return;
}
END_METHOD

BEGIN_METHOD_VOID ( COPTIONS_free )
{
  int i = 0;
  
  GB.FreeString(&(THIS->options));
  
  for (i = 0; i < GB.Count(THIS->argv); i++) 
  {
    GB.FreeString(&(THIS->argv[i]));
  }
  GB.FreeArray((void *) &(THIS->argv)); 
  
  if(THIS->rest!=NULL)
  {
  	  GB.Unref((void **)&THIS->rest);
  }
  
  for (i = 0; i < GB.Count(THIS->opt_arg); i++) 
  {
  	GB.FreeString(&(THIS->opt_arg[i]));
  }
  GB.FreeArray((void *) &(THIS->opt_arg));
  
  for (i = 0; i < GB.Count(THIS->opt_found); i++) 
  {
	GB.FreeString(&(THIS->opt_found[i]));
  }
  GB.FreeArray((void *) &(THIS->opt_found)); 
  
  for (i = 0; i < GB.Count(THIS->invalid); i++) 
  {
	GB.FreeString(&(THIS->invalid[i]));
  }
  
  if(THIS->return_temp!=NULL)
  {
	GB.Unref(&THIS->return_temp);
  }
  
  GB.FreeArray((void *) &(THIS->invalid)); 
  GB.Unref((void *)&THIS->cmdline);
}
END_METHOD

GB_DESC GetOptionsDesc[] =
{
  GB_DECLARE("GetOptions", sizeof(COPTIONS)),

  GB_METHOD ( "_new", NULL, COPTIONS_new, "(Options)s[(Argument)String[];]" ),
  GB_METHOD ( "_free", NULL, COPTIONS_free, NULL ),
  GB_METHOD ( "_get" ,"v",COPTIONS_get,"(Option)s" ),
  GB_METHOD ( "NextValidOption","s",COPTIONS_next,NULL ),
  GB_METHOD ( "OptionArgument","s",COPTIONS_opt_arg,NULL ),
  GB_METHOD ( "RestOfArgs","String[]",COPTIONS_rest,NULL ),
  GB_METHOD ( "GetErrorOption","s",COPTIONS_invalid,NULL ),
  GB_METHOD ( "GetArgument","String[]",COPTIONS_getarg,"(Option)s"),
  GB_METHOD ( "GetAllOptions","String[]",COPTION_getallopt,NULL),

  //GB_PROPERTY ( "Argument", "String[]", COPTIONS_arg),
  //GB_STATIC_PROPERTY ( "Options", "s",COPTIONS_options),
  GB_PROPERTY_READ ("CommandLineArgs","String[]",COPTIONS_cmdline),

  GB_END_DECLARE
};
