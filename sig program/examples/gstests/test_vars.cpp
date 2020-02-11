/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/


# include <sig/gs.h>
# include <sig/gs_var.h>
# include <sig/gs_vars.h>
# include <sig/gs_string.h>

GsVar vb("vb.a",'b'), vi("vi",'i'), vf("vf",'f'), vs("vs",'s');

char data[] = "table [ vb.a=true true; vi=1 2; vf=-2.0; vs = aa bb; ]";

static void out ()
 {
   gsout << vb << gsnl;
   gsout << vi << gsnl;
   gsout << vf << gsnl;
   gsout << vs << gsnl;
   gsout << gsnl;
 }

void checkio ()
 {
   GsString buf;
   GsOutput o; o.init(buf);
   o << vb << vi << vf << vs;
   GsInput i; i.init(buf);
   i >> vb >> vi >> vf >> vs;
 }

static void test1 ()
 {
   vb.push(true); vb.push(true); vb.push(false);
   vi.push(-10); vi.push(20); vi.push(-200);
   vf.push(-33.3); vf.push(2.314);
   vs.push("red"); vs.push("green"); vs.push("blue");

   out ();
   checkio ();
   out ();

   GsVars vt;
   vt.add ( new GsVar(vb) );
   vt.add ( new GsVar(vi) );
   vt.add ( new GsVar(vb) );
   vt.add ( new GsVar(vb) );
   vt.add ( new GsVar(vf) );
   vt.add ( new GsVar(vs) );

   gsout << vt << gsnl;

   vt.remove ( vt.search("vb.a") );
   vt.remove ( vt.search("vb.a") );

   gsout << vt << gsnl;

   GsInput in; in.init(data);
   in >> vt;
   gsout << vt << gsnl;
 }

static void test2 ()
 {
   GsVars v;

   v.add ( "image.size", 640 ); v.set ( "image.size", 480, 1 );
   v.add ( "image.color", true );
   v.add ( "distance", 456.34f );
   v.add ( "pi", 3.141592654f );
  
   GsString buf;
   GsOutput o;
   o.init(buf);
   o<<v;
   gsout<<"Output to string:\n"<<buf<<gsnl;

   gsout<<"Reading back...\n";
   GsVars v2;
   GsInput in;
   in.init(buf);
   in>>v2;
   gsout<<"Result:\n";
   gsout<<v2<<gsnl;

   v2.name ( "user data" );
   gsout<<"\nWith a name:\n";
   gsout<<v2<<gsnl;
 }

void test_vars ()
 {
   test1();
   test2();
 }
