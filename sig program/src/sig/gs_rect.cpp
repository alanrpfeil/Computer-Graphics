/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_rect.h>

//================================== GsRect =========================================

void GsRect::round_coordinates ()
 { 
   x=(float)gs_round(x);
   y=(float)gs_round(y);
   w=(float)gs_round(w);
   h=(float)gs_round(h);
 }

void GsRect::get_vertices ( GsPnt2& a, GsPnt2& b, GsPnt2& c, GsPnt2& d ) const
 {
   a.set ( x,	y	);
   b.set ( xp(), y	);
   c.set ( x,	yp() );
   d.set ( b.x,  c.y  );
 }

//================================ End of File =================================================


