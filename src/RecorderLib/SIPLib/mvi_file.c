#include "mvi_file.h"


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void read_file(const char* pname,char* pbuf,int* len)
{
	FILE* fl;
	int l;

	fl= fopen(pname,"rb");
	if ( fl <= 0 ) return ;

	l = fread(pbuf,sizeof(char),*len,fl);
	fclose(fl); 
	*len = l;
	if ( !l ) return ;
	return ;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void write_file(const char* pname,const char* pbuf,int len)
{

	FILE* fl;
	int l;
	fl= fopen(pname,"ab");
	if ( fl <= 0 ) return ;
	//printf(" fl = (%u)",fl);

	//l = fwrite("\n",sizeof(char),1,fl);

	l = fwrite(pbuf,sizeof(char),len,fl);
	fclose(fl); 
	len = l;
	if ( l != len ) return ;
	return ;

}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void write_file_flush(const char* pname,char* pbuf,int len,FILE** fl)
{
	if ( !*fl ) { 
		*fl= fopen(pname,"ab");
		if ( *fl <= 0 ) { 	*fl = NULL; return ; }
	}
	fwrite(pbuf,sizeof(char),len,*fl);
	fflush(*fl);
}