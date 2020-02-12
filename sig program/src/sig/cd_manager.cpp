/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/cd_manager.h> 

//============================ DefaultImplementation =====================================

CdImplementation* CdDefAllocator ()
{
	return new CdImplementation; // empty implementaiton
}

static CdImplementation* (*CdCurAllocator)() = CdDefAllocator;

void CdManager::set_implementation_allocator (  CdImplementation* (*f)() )
{
	CdCurAllocator = f;
}

//================================ CdManager =====================================

CdManager::CdManager ( CdImplementation* cdi )
{
	if ( !cdi ) cdi = CdCurAllocator();
	_cdi = cdi;
	_cdi->ref();
}
  
CdManager::~CdManager ()
{
	_cdi->unref();
}

void CdManager::set_implementation ( CdImplementation* cdi )
{
	_cdi->unref();
	_cdi = cdi;
	_cdi->ref ();
}

//=================================== EOF =====================================
