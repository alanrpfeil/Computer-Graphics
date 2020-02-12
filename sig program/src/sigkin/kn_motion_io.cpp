/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# include <stdlib.h>
# include <sigkin/kn_motion.h>
# include <sigkin/kn_skeleton.h>
# include <sig/gs_string.h>
# include <sig/gs_euler.h>

//# define GS_USE_TRACE2 // save bvh
//# define GS_USE_TRACE4 // save
# include <sig/gs_trace.h>

static bool hasTranslation(KnJoint *j);
static bool hasRotation(KnJoint *j);

/* MotionBuilder friendly level:
// 0: not friendly; 1: omit channels for child-less joint; 2: also rename child-less joint to "End Site" */
static void write_bvh_joint(GsOutput &out, KnJoint *j, int marg, unsigned int MB_friendly);

//============================= load bvh ============================

static inline void sGetRot ( int order, GsQuat& q, float a1, float a2, float a3 )
 {
   if ( order==123 )	  q = GsQuat(GsVec::i,a1) * GsQuat(GsVec::j,a2) * GsQuat(GsVec::k,a3);
   else if ( order==132 ) q = GsQuat(GsVec::i,a1) * GsQuat(GsVec::k,a2) * GsQuat(GsVec::j,a3);
   else if ( order==213 ) q = GsQuat(GsVec::j,a1) * GsQuat(GsVec::i,a2) * GsQuat(GsVec::k,a3);
   else if ( order==231 ) q = GsQuat(GsVec::j,a1) * GsQuat(GsVec::k,a2) * GsQuat(GsVec::i,a3);
   else if ( order==312 ) q = GsQuat(GsVec::k,a1) * GsQuat(GsVec::i,a2) * GsQuat(GsVec::j,a3);
					 else q = GsQuat(GsVec::k,a1) * GsQuat(GsVec::j,a2) * GsQuat(GsVec::i,a3);
 }

bool KnMotion::load_bvh ( GsInput& in )
 {
   GsInput::TokenType type;

   GsString name;
   float freq = 0.0f;
   int c, n, frames = 0;
   int euleri=0, eulero=0;
   GsArray<int> eulerorder;

   // 1. read channels
   KnChannels* chs = new KnChannels;

   while (true)
	{ type = in.get();
	  if ( in.end() ) { chs->unref(); return false; }
	  const GsString& s = in.ltoken();

	  if ( type==GsInput::String )
	   { if ( s=="ROOT" || s=="JOINT" )
		  { if ( in.get()!=GsInput::String ) { delete chs; return false; }
			 name = in.ltoken();
			 euleri = 100;
			 eulero = 0;
		  }
		 else if ( s=="CHANNELS" )
		  { if ( in.get()!=GsInput::Number ) { delete chs; return false; }
			n = in.ltoken().atoi();

			while ( n-- )
			 { in.get();
			   c = KnChannel::type(in.ltoken());
			   if (c == KnChannel::Invalid) {
				   chs->unref();
				   return false;
			   }
			   // we now read rotations in any given order, euleri encodes the order
			   if ( c>=KnChannel::XRot && c<=KnChannel::ZRot ) // rotation channel
				{
				  if ( c==KnChannel::XRot ) eulero += 1*euleri;
				   else if ( c==KnChannel::YRot ) eulero += 2*euleri;
						 else eulero += 3*euleri; //KnChannel::ZRot
				  if ( KnSkeleton::ConvertBvhToQuat )
				   { if ( euleri==1 ) c=KnChannel::Quat; else c=-1;
				   }
				  euleri /= 10;
				}

			   if ( c>=0 )
				{ chs->add ( KnJointName(name), (KnChannel::Type)c );
				  eulerorder.push() = eulero;
				}
			  }
		  }
		 else if ( s=="MOTION" ) break;
	   }
	}

	// No skeleton is read, abort.
	if(chs->size() == 0)
		return false;

   // 2. read motion description
   while ( in.get()==GsInput::String )
	{ if ( in.ltoken()=="Frames" )
	   { in.getc();
		 frames = in.geti();
	   }

	  if ( in.ltoken()=="Time" )
	   { in.getc();
		 freq = in.getf();
		 _freq = freq;
	   }
	}
   in.unget();

   if ( chs->size()==0 ) { delete chs; return false; }

   _frames.capacity ( frames );
   _frames.size ( 0 );
   int chsize = chs->size();

   // 3. Read the motion data in bvh format
   int f, i, j;
   float val, kt = 0;
   for ( f=0; f<frames; f++ )
	{ insert_frame ( f, kt, new KnPosture(chs) );
	  kt += freq;
	  for ( j=0, i=0; i<chsize; i++ )
	   { c = chs->get(i).type();
		 if ( c<=KnChannel::ZPos )
		  { val = in.getf();
			_frames.top().posture->values[j++] = val;
		  }
		 else if ( c<=KnChannel::ZRot )
		  { val = in.getf();
			val = GS_TORAD(val);
			_frames.top().posture->values[j++] = val;
		  }
		 else if ( c==KnChannel::Quat ) // convert the ZXY to quat format
		  { GsQuat q;
			float a1=in.getf(); a1=GS_TORAD(a1);
			float a2=in.getf(); a2=GS_TORAD(a2);
			float a3=in.getf(); a3=GS_TORAD(a3);
			sGetRot ( eulerorder[i], q, a1, a2, a3 );
			q.normalize();
			_frames.top().posture->values[j++] = q.w;
			_frames.top().posture->values[j++] = q.x;
			_frames.top().posture->values[j++] = q.y;
			_frames.top().posture->values[j++] = q.z;
		  }
		 else
		  { _frames.top().posture->values[j++] = 0;
		  }
	   }
	}

   compress ();

   return true;
 }

//============================= load ============================

bool KnMotion::load ( const char* filename )
 {
   //GS_TRACE3("Load from file...");
   GsInput in;
   if ( !in.open(filename) ) return false;
   if ( !load(in) ) return false;
   return true;
 }

bool KnMotion::load ( GsInput& in )
 {
   in.lowercase ( false ); // string comparison remains case insensitive
   in.commentchar ( '#' );

   // init and set new filename
   init ();
   if ( in.filename() ) filename(in.filename());

   // 1. verify signature
   in.get();
   if ( in.ltoken()=="HIERARCHY" ) // bvh format
	{ GsString s = in.filename();
	  remove_path ( s );
	  remove_extension ( s );
	  name ( s );
	  return load_bvh ( in );
	}
   else if ( in.ltoken()!="KnMotion" && in.ltoken()!="KnMotion" )
	{ return false;
	}

   // 2. read name if any:
   in.get();
   if ( in.ltoken()=="name" )
	{ in.get(); name ( in.ltoken() ); }
   else
	in.unget();

   // 3. read channels
   KnChannels* chs = new KnChannels;
   in >> *chs;

   // 4. check if a start_kt is set
   float startkt = -1.0f;
   if ( in.check()==GsInput::String )
	{ in.get();
	  if ( in.ltoken()=="startkt" || in.ltoken()=="start_kt" )
	   { in.get();
		 startkt = in.ltoken().atof();
	   }
	  else in.unget();
	}

   // 5. read number of frames, if specified
   int numframes = -1;
   in.get();
   if ( in.ltoken()=="frames" )
	{ in.get();
	  numframes = in.ltoken().atoi();
	  _frames.size ( numframes );
	  _frames.size ( 0 );
	}
   else
	in.unget();

   // 6. read data
   int f=0;
   while ( !in.end() )
	{ if ( in.get()==GsInput::End ) break; // kt
	  if ( in.get()==GsInput::End ) break;
	  insert_frame ( f, in.ltoken().atof(), new KnPosture(chs) );
	  in.get(); // fr
	  in >> (*_frames[f].posture);
	  f++;
	  if ( f==numframes ) break;
	}

   // 7. read user data, if specified
   while( !in.end() )
	{ if ( in.get()==GsInput::End ) break;
	  if ( in.ltoken()=="userdata" ) in >> *_userdata;
	}

   if ( startkt>-1 ) move_keytimes ( startkt );

   compress ();

   return true;
 }

//============================= save ============================

bool KnMotion::save ( const char* filename )
 {
   GsOutput out;

   GS_TRACE4 ( "Opening [" << filename << "]...\n" );
   if ( !out.open(filename) ) return false;

   GS_TRACE4("Saving...");
   return save(out);
 }

bool KnMotion::save ( GsOutput& out )
 {
   out << "KnMotion\n\n";

   if ( _name )
	{ out << "name " << GsSafeWrite(_name) << gsnl << gsnl;
	}

   KnChannels* chs = channels();
   if ( !chs ) return true;

   out<<"# Reminder: Quat channels are stored in axis-angle format\n\n";

   out << (*chs) << gsnl;

   out << "frames " << _frames.size() << gsnl;

   int i;
   /*chsize = */ chs->size();
   for ( i=0; i<_frames.size(); i++ )
	{ out << "kt " << _frames[i].keytime << " fr ";
	  _frames[i].posture->output(out,false,true);
	}
   out << gsnl;

   if ( _userdata ) out << "userdata" << gsnl << *_userdata << gsnl;

   return true;
 }

/* MotionBuilder friendly level:
// 0: not compatible; 1: omit channels for child-less joint; 2: also rename child-less joint to "End Site"
// Tested on MotionBuilder 2012. note: in certain cases level 2 is required for bvh import to MB. */
static void write_bvh_joint ( GsOutput &out, KnJoint *j, int marg, unsigned int MB_friendly )
{
	out.margin_char(gspc);
	out.fmtfloat("%f");

	out.margin(marg);

	out.outm();out.outm(); // by default bvh uses two spaces as margin

	if(MB_friendly==2&&j->children()==0)
		out << "End Site" << gsnl;
	else
	{
		if (j->parent())
			out << "JOINT ";
		else
			out << "ROOT ";

		out << j->name() << gsnl;
	}

	// Write offset.
	out.outm();out.outm();
	out << "{" << gsnl;

	out.margin(marg + 1);

	out.outm();out.outm(); // by default bvh uses two spaces as margin
	out << "OFFSET " << j->offset() << gsnl;

	if(j->children()==0 && MB_friendly>0) { } // don't write anything here
	else if ((!j->parent()) || (hasRotation(j) && hasTranslation(j))) // Root is set to have 6 channels.
	{
		out.outm();out.outm();
		out << "CHANNELS" << gspc
			<< "6 Xposition Yposition Zposition" << gspc
			<< "Zrotation Xrotation Yrotation" << gsnl;
	}
	else
	{
		out.outm();out.outm();
		out << "CHANNELS" << gspc
			<< "3 Zrotation Xrotation Yrotation" << gsnl;
	}

	// Recurse into children.
	for (int i = 0; i < j->children(); i++)
		write_bvh_joint(out, (KnJoint*) j->child(i), marg + 1, MB_friendly);

	out.margin(marg);
	out.outm();out.outm();
	out << '}' << gsnl;
}


// Added by David Huang, Apr 2010; modified on June 2012.
bool KnMotion::save_bvh ( const char *filename, int fps, unsigned int MB_friendly )
{
	GS_TRACE2("Saving...");

	KnSkeleton *sk = skeleton();

	if (!sk)
	{
		gsout << "No skeleton is attached to this motion, aborting..."<<gsnl;
		return false;
	}

	KnChannels *chs = channels();
	if (!chs || chs->size() == 0)
		return false;

	GsOutput out;
	out.fmtfloat("f");

	GS_TRACE2("Opening [" << filename << "]...\n");
	if (!out.open(filename)) {
		out.close();
		return false;
	}

	out << "HIERARCHY" << gsnl;

	const GsArray<KnJoint*> &_joints = sk->joints();

	write_bvh_joint(out, _joints[0], 0, MB_friendly);

	out << "MOTION\n";

	float total_time = _frames.top().keytime;

	int total_frames = 0; // to get the actual total frames
	for ( ;(float)total_frames/fps < total_time; total_frames++) {}

	out << "Frames:" << gspc << total_frames << gsnl;
	out << "Frame Time:" << gstab << 1.0f / fps << gsnl;

	GS_TRACE2("Skeleton name: " << sk->name());

	float x, y, z;
	int i, k, jsize = _joints.size();

	for (i = 0; (float)i/fps <= total_time; i++) {
		this->apply((float)i/fps);

		for(k=0; k<jsize; k++)
		{
			KnJoint* j = _joints[k];
			if(MB_friendly>0 && j->children()==0) // don't write anything here
				continue;
			else if ((!j->parent()) || (hasRotation(j) && hasTranslation(j))) // Root is set to have 6 channels.
			{
				GsVec pos = j->pos()->value();
				out << pos.x << gspc << pos.y << gspc << pos.z << gspc;
				gs_angles(gsYXZ, j->quat()->value(), x, y, z);
				out << GS_TODEG(z) << gspc
					<< GS_TODEG(x) << gspc
					<< GS_TODEG(y) ;
			}
			else // Print Rotation
			{
				gs_angles(gsYXZ, j->quat()->value(), x, y, z);
				out << GS_TODEG(z) << gspc
					<< GS_TODEG(x) << gspc
					<< GS_TODEG(y) ;
			}
			if(k < jsize - 1)
				out << gspc;
		}
		out << gsnl;
	}

	out << gsnl;

	return true;
}

/*
 * Returns true if this joint does not have translation channels.
 * It has to be connected to the skeleton in order to get the channels.
 * If not connected to skeleton false is returned.
 *
 * Mentar Mahmudi - Jan, 2010.
 */

static bool hasRotation(KnJoint *j)
{
	KnChannels &chs = j->skeleton()->channels();
	for (int i = 0; i < chs.size(); i++)
		if (chs[i].jname() == j->name() &&
			chs[i].type() >= KnChannel::XRot &&
			chs[i].type() <= KnChannel::Twist)
				return true;

	return false;
}

static bool hasTranslation(KnJoint *j)
{
	KnChannels &chs = j->skeleton()->channels();
	for (int i = 0; i < chs.size(); i++)
		if (chs[i].jname() == j->name() && chs[i].type() <= KnChannel::ZPos)
				return true;

	return false;
}


//============================ End of File ===========================
