/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/cd_implementation.h>

//================================ CdImplementation =====================================

CdImplementation::CdImplementation ()
{
}

CdImplementation::~CdImplementation ()
{
}

void CdImplementation::init ()
{ 
}

int CdImplementation::insert_object ( const GsModel& m )
{
	return 0;
}

void CdImplementation::remove_object ( int id )
{
}

bool CdImplementation::id_valid ( int id )
{
	return false;
}

void CdImplementation::update_transformation ( int id, const GsMat& m )
{
}

void CdImplementation::activate_object ( int id )
{
}

void CdImplementation::deactivate_object ( int id )
{
}

void CdImplementation::activate_pair ( int id1, int id2 )
{
}
	
void CdImplementation::deactivate_pair ( int id1, int id2 )
{
}

bool CdImplementation::pair_deactivated ( int id1, int id2 )
{
	return true;
}

int CdImplementation::count_deactivated_pairs ()
{
	return 0;
}

const GsArray<int>& CdImplementation::colliding_pairs () const
{
	return _pairs;
}

bool CdImplementation::collide ()
{
	return false;
}

bool CdImplementation::collide_all ()
{
	return false;
}

bool CdImplementation::collide_tolerance ( float toler )
{
	return false;
}

//=================================== EOF =====================================

