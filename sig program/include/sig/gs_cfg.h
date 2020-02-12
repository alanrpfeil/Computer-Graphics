/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_cfg.h 
 * A 6-DOF configuration specified by a vector and a quaternion */

# ifndef GS_CFG_H
# define GS_CFG_H

# include <sig/gs_vec.h> 
# include <sig/gs_quat.h> 

/*! \class GsCfg gs_cfg.h
	A 6-DOF configuration specified by a vector (the position) and a quaternion (the orientation).
	GsCfg is a simple class to facilitate 6-DOF placement computation with a vector and a quaternion. */
class GsCfg
{  public :
	GsPnt p; // reminder: GsPnt is the same as GsVec
	GsQuat q;

   public :
	static const GsCfg null;

   public :

	/*! Initializes GsCfg at zero position and null rotation. Implemented inline. */
	GsCfg () {}

	/*! Copy constructor. Implemented inline. */
	GsCfg ( const GsCfg& l ) : p(l.p), q(l.q) {}

	/*! Initializes from a vector and quaternion.*/
	GsCfg ( const GsVec& lp, const GsQuat& lq ) : p(lp), q(lq) {}

	/*! Initializes from a matrix.*/
	GsCfg ( const GsMat& m) { set(m); }

	/*! Set from another GsCfg. Implemented inline. */
	void set ( const GsCfg& l ) { p=l.p; q=l.q; }

	/*! Set from a vector and quaternion.*/
	void set ( const GsVec& lp, const GsQuat& lq ) { p=lp; q=lq; }

	/*! Set (p,q) from given matrix, in column major format only. */
	void set ( const GsMat& m ) { p.set(m.e41,m.e42,m.e43); mat2quat(m,q); }

	/*! Returns the transformation matrix with translation v and rotation q, in column major format only. */
	void compose ( GsMat& m ) const { ::compose(q,p,m); }

	/*! Returns true if the position and quaternion are zero/null. */
	bool isnull () const { return p==GsVec::null && q==GsQuat::null; }

	/*! Comparison operator. Implemented inline. */
	friend bool operator == ( const GsCfg& l1, const GsCfg& l2 ) { return l1.p==l2.p && l1.q==l2.q; }

	/*! Comparison operator. Implemented inline. */
	friend bool operator != ( const GsCfg& l1, const GsCfg& l2 ) { return l1.p!=l2.p || l1.q!=l2.q; }

	/*! Returns the position linear interpolation and the quaternion slerp. 
		Parameter l1 is not const because its quaternion is normalized. */
	friend GsCfg interp ( GsCfg& l1, const GsCfg& l2, float t )
		   { return GsCfg ( mix(l1.p,l2.p,t), slerp(l1.q,l2.q,t) ); }

	/*! Output in format: "x y z, ax ay az ang" */
	friend GsOutput& operator<< ( GsOutput& o, const GsCfg& l ) { return o << l.p <<", "<< l.q; }

	/*! Input from format: "x y z, ax ay az ang". */
	friend GsInput& operator>> ( GsInput& in, GsCfg& l ) { in >> l.p; in.get(); in >> l.q; return in; }
};

/*! Sum of the vector distance and the quaternion distance multiplied by wq */
inline float dist ( GsCfg& l1, const GsCfg& l2, float qw )
	   { return dist(l1.p,l2.p) + dist(l1.q,l2.q,qw); }

inline void mat2cfg ( const GsMat& m, GsCfg& l ) { l.set(m); }
inline void cfg2mat ( const GsCfg& l, GsMat& m ) { l.compose(m); }

//============================== end of file ===============================

# endif // GS_CFG_H
