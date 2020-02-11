/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs.h>
# include <sig/gs_output.h>

template <typename X>
void test ( const char* name, unsigned bytes1, unsigned bytes2 )
{
	unsigned xsize = (unsigned)sizeof(X);
	gsout << name << " = " << xsize << (xsize==1?" byte":" bytes"); 
	if ( xsize==bytes1 || xsize==bytes2 )
	{	gsout<<" - OK ";
		if ( bytes1!=bytes2 ) gsout<<"("<<bytes1<<" or "<<bytes2<<")";
		gsout<<gsnl;
	}
	else
	{	gsout<<" - ERROR! should have "<<bytes1;
		if ( bytes1!=bytes2 ) gsout << " or "<<bytes2;
		gsout<<"!\n";
	}
}

void test_types ()
{
	int* ip;

	gsout << "Executable platform:\n";
	# ifdef GS_64BITS
	gsout << "64 bits.\n\n";
	# else
	gsout << "32 bits.\n\n";
	# endif

	gsout << "SIG Types:\n";
	test<gschar>   ( "gschar  ", 1, 1 ); //!< 1 byte signed int, from -127 to 128
	test<gsbyte>   ( "gsbyte  ", 1, 1 ); //!< 1 byte unsigned int, from 0 to 255
	test<gscbool>  ( "gscbool ", 1, 1 ); //!< 1 byte char type intended to store a boolean value
	test<gscenum>  ( "gscenum ", 1, 1 ); //!< 1 byte signed char type intended to store an enumerator
	test<gsint>	   ( "gsint   ", 4, 8 ); //!< 4 or 8 bytes int depending on the platform
	test<gsuint>   ( "gsuint  ", 4, 8 ); //!< 4 or 8 bytes unsigned int depending on the platform
	test<gsuint16> ( "gsuint16", 2, 2 ); //!< 2 bytes unsigned int, from 0 to 65,535
	test<gsint16>  ( "gsint16 ", 2, 2 ); //!< 2 bytes integer, from -32,768 to 32,767
	test<gsuint32> ( "gsuint32", 4, 4 ); //!< 4 bytes unsigned int, from 0 to 4294967295
	test<gsint32>  ( "gsint32 ", 4, 4 ); //!< 4 bytes signed integer, from -2147483648 to 2147483647
	test<gsintp>   ( "gsintp  ", sizeof(ip), sizeof(ip) );//!< integer type with same size as a memory pointer: 32 or 64 bits

	gsout << "\nOther Types:\n";
	test<size_t>  ( "size_t ", 4, 8 );
}
