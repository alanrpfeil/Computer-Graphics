/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sig/gs_matn.h>
# include <sig/gs_output.h>

//# define GS_USE_TRACE1 // const/dest
# include <sig/gs_trace.h>

// buffers:
static GsBuffer<int>* IBuff=0;
static GsBuffer<double>* DBuff=0;

// macros:
# define IBUFF(ib,s) if(!IBuff)IBuff=new GsBuffer<int>; if(IBuff->size()<s)IBuff->size(s); ib=&(*IBuff)[0]; 
# define DBUFF(db,s) if(!DBuff)DBuff=new GsBuffer<double>; if(DBuff->size()<s)DBuff->size(s); db=&(*DBuff)[0]; 
# define MAT(m,n) _data[_col*m+n]

GsMatn::GsMatn () : _lin(0), _col(0)
 {
   GS_TRACE1 ("Default Constructor");
 }

GsMatn::GsMatn ( const GsMatn& m ) : _lin(m._lin), _col(m._col)
 {
   GS_TRACE1 ("Copy Constructor\n");
   _data = m._data;
 }

GsMatn::GsMatn ( int m, int n ) : _lin(m), _col(n)
 {
   GS_TRACE1 ("Size Constructor\n");
   _data.size ( _lin*_col );
 }

/*GsMatn::~GsMatn ()
 {
   GS_TRACE1 ("Destructor\n");
 }*/

void GsMatn::size ( int m, int n )
 {
   _data.size ( m*n );
   _lin=m; _col=n;
 }

void GsMatn::resize ( int m, int n )
 {
   int lin = GS_MIN(m,_lin);
   int col = GS_MIN(n,_col);
   GsMatn mat ( m, n );

   int i, j;
   for ( i=0; i<lin; i++ )
	for ( j=0; j<col; j++ )
	  mat(i,j) = _data[_col*j+i];

   for ( i=lin; i<m; i++ )
	for ( j=col; j<n; j++ )
	  mat(i,j) = 0;

   adopt ( mat );
 }

void GsMatn::submat ( const GsMatn &m, int li, int le, int ci, int ce )
 {
   size ( le-li+1, ce-ci+1 );

   for ( int i=li; i<=le; i++ )
	for ( int j=ci; j<=ce; j++ )
	  set ( i-li, j-ci, m.get(i,j) );
 }

void GsMatn::submat ( int l, int c, const GsMatn &m,  int li, int le, int ci, int ce )
 {
   for ( int i=li; i<=le; i++ )
	for ( int j=ci; j<=ce; j++ )
	  set ( l+i-li, c+j-ci, m.get(i,j) );
 }

void GsMatn::column ( const GsMatn &m, int s )
 {
   size ( m.lin(), 1 );

   for ( int i=0; i<_lin; i++ )
	 set ( i, 0, m.get(i,s) );
 }

void GsMatn::column ( int c, const GsMatn &m, int mc )
 {
   for ( int i=0; i<_lin; i++ )
	 set ( i, c, m.get(i,mc) );
 }

void GsMatn::identity ()
 {
   int i, s=_data.size(), d=_col+1;
   for ( i=0; i<s; i++ )
	_data[i] = i%d? 0.0:1.0;
 }

void GsMatn::transpose ()
 {
   int i, j;

   if ( _lin==_col )
	{ double tmp;
	  for ( i=0; i<_lin; i++ )
	   for ( j=i+1; j<_col; j++ )
		 GS_SWAP ( _data[_col*i+j], _data[_col*j+i] );
	}
   else if ( _lin==1 || _col==1 )
	{ int tmp;
	  GS_SWAP ( _lin, _col );
	}
   else
	{ GsMatn m ( _col, _lin );
	  for ( i=0; i<_lin; i++ )
	   for ( j=0; j<_col; j++ )
		m(j,i) = get(i,j);
	  adopt ( m );
	}
 }

void GsMatn::swaplines ( int l1, int l2 )
 {
   int i;
   double tmp, *p1, *p2;
   p1 = &_data[0]+(_col*l1);
   p2 = &_data[0]+(_col*l2);
   for ( i=0; i<_col; i++ ) GS_SWAP ( p1[i], p2[i] ); 
 }

void GsMatn::swapcolumns ( int c1, int c2 )
 {
   int i;
   double tmp;
   for ( i=0; i<_lin; i++ ) GS_SWAP ( MAT(i,c1), MAT(i,c2) ); 
 }

void GsMatn::random ( double inf, double sup )
 {
   int i=_data.size();
   while ( i ) _data[--i] = gs_random(inf,sup);
 }

void GsMatn::random ( float inf, float sup )
 {
   int i=_data.size();
   while ( i ) _data[--i] = (double)gs_random(inf,sup);
 }

double GsMatn::norm () const
 {
   if ( !_data ) return 0.0;

   int s = size();
   double sum = 0.0;
   for ( int i=0; i<s; i++ ) sum += _data[i]*_data[i];

   return sqrt ( sum );
 }

void GsMatn::add ( const GsMatn& m1, const GsMatn& m2 )
 { 
   size ( m1.lin(), m1.col() );
   int s = size();
   for ( int i=0; i<s; i++ ) _data[i] = m1._data[i] + m2._data[i];
 }

void GsMatn::sub ( const GsMatn& m1, const GsMatn& m2 )
 {
   size ( m1.lin(), m1.col() );
   int s = size();
   for ( int i=0; i<s; i++ ) _data[i] = m1._data[i] - m2._data[i];
 }

void GsMatn::mult ( const GsMatn& m1, const GsMatn& m2 )
 {
   int l, c, i, j, k, klast; 
   double sum;

   GsMatn *m = (&m1==this || &m2==this)? new GsMatn : this;

   l = m1._lin;
   c = m2._col;
   m->size(l,c);

   klast = GS_MIN(m1._col,m2._lin);
   for ( i=0; i<l; i++ )
	for ( j=0; j<c; j++ )
	 { sum = 0;
	   for ( k=0; k<klast; k++ ) sum += m1.get(i,k) * m2.get(k,j);
	   m->set(i,j,sum);
	 }

   if ( m!=this ) { *this=*m; delete m; }
 }

void GsMatn::abandon ( GsBuffer<double>& buf )
 {
   buf.adopt ( _data );
   _lin = _col = 0;
 }

void GsMatn::adopt ( GsMatn& m )
 {
   if ( this==&m ) return;
   _data.adopt ( m._data );
   _lin  = m._lin;  m._lin=0;
   _col  = m._col;  m._col=0;
 }

void GsMatn::adopt ( GsBuffer<double>& buf, int m, int n )
 {
   _data.adopt ( buf );
   _lin = m;
   _col = n;
 }

//============================ Operators =========================================

void GsMatn::operator = ( const GsMatn& m )
 {
   if ( this != &m )
	{ _data = m._data;
	  _lin  = m._lin;
	  _col  = m._col;
	}
 }

void GsMatn::operator += ( const GsMatn& m )
 {
   int i = size();
   while ( --i>=0 ) _data[i]+=m.get(i);
 }

void GsMatn::operator -= ( const GsMatn& m )
 {
   int i = size();
   while ( --i>=0 ) _data[i]-=m.get(i);
 }

void GsMatn::operator *= ( float s )
 {
   int i = size();
   while ( --i>=0 ) _data[i] *= s;
 }

//================================= friends ==================================

GsOutput& operator << ( GsOutput &o, const GsMatn &m )
 {
   for ( int i=0; i<m.lin(); i++ )
	{ for ( int j=0; j<m.col(); j++ )
	   { o<<m.get(i,j)<<gspc; }
	  o<<gsnl;
	}
   return o;
 }

double dist ( const GsMatn &a, const GsMatn &b )
 {
   GsMatn m ( a.lin(), a.col() );
   m.sub ( a, b );
   return m.norm();
 }

# define TINY 1.0e-20;

const int *ludcmp ( GsMatn &a, double *d, bool pivoting )
 {
   int i, j, k, imax=0;
   double big, sum, tmp;
   int n=a._lin;
   double *vv=0;	// vv stores the implicit scaling of each row
   int *indx=0;	 // row permutation buffer

   DBUFF ( vv, n );   // use static buffer for that
   IBUFF ( indx, n ); // use static buffer for that

   if (d) *d=1.0; // no row interchanges yet

   for ( i=0; i<n; i++ ) // loop over rows to get the scaling
	{ big = 0.0;
	  for ( j=0; j<n; j++ ) { tmp=GS_ABS(a(i,j)); if ( tmp>big ) big=tmp; }
	  if ( big==0.0 ) { gsout.warning("Singular matrix in routine ludcmp"); return 0; }
	  vv[i]=1.0/big; // save the scaling
	}

   for ( j=0; j<n; j++ ) // loop over columns of the Crout's method
	{ for ( i=0; i<j; i++ ) 
	   { sum = a(i,j);
		 for ( k=0; k<i; k++ ) sum -= a(i,k)*a(k,j);
		 a(i,j)=sum;
	   }
	  big = 0.0; // search for largest pivot element
	  for ( i=j; i<n; i++ ) 
	   { sum = a(i,j);
		 for ( k=0; k<j; k++ ) sum -= a(i,k)*a(k,j);
		 a(i,j) = sum;
		 tmp = vv[i]*GS_ABS(sum);
		 if ( tmp>=big) { big=tmp; imax=i; }
	   }
	  
	  if ( pivoting )
	   { if ( j!=imax ) // interchange rows if needed
		  { for ( k=0; k<n; k++ ) GS_SWAP ( a(imax,k), a(j,k) );
			if (d) *d = -*d;
			vv[imax]=vv[j];
		  }
		 indx[j]=imax;
	   }
	  else indx[j]=j;

	  if ( a(j,j)==0.0 ) a(j,j)=TINY;
	  if ( j!=n-1 )
	   { tmp = 1.0/a(j,j);
		 for ( i=j+1; i<n; i++ ) a(i,j) *= tmp;
	   }
	}
   return indx;
 }

bool ludcmp ( const GsMatn &a, GsMatn &l, GsMatn &u )
 {
   u = a;
   const int *indx = ludcmp(u,0,false);
   if ( !indx ) return false;

   int n = a.lin();
   l.size ( n, n );

   for ( int i=0; i<n; i++ )
	for ( int j=0; j<n; j++ )
	 { if ( i>j ) { l(i,j)=u(i,j); u(i,j)=0.0; }
		else { l(i,j) = i==j? 1.0:0.0; }
	 }

   return true;
 }

void lubksb ( const GsMatn &a, GsMatn &b, const int *indx )
 {
   int i, ii=-1, ip, j;
   double sum;
   int n=a.lin();

   for ( i=0; i<n; i++ )
	{ ip = indx[i];
	  sum = b[ip];
	  b[ip] = b[i];
	  if (ii>=0) { for ( j=ii; j<=i-1; j++ ) sum -= a.get(i,j)*b[j]; }
	   else if (sum) ii=i;
	  b[i]=sum;
	}

   for ( i=n-1; i>=0; i-- ) 
	{ sum = b[i];
	  for ( j=i+1; j<n; j++ ) sum -= a.get(i,j)*b[j];
	  b[i] = sum/a.get(i,i);
	}
 }

bool lusolve ( GsMatn &a, GsMatn &b )
 {
   const int *indx = ludcmp ( a );
   if ( !indx ) return false;
   lubksb ( a, b, indx );
   return true;
 }

bool lusolve ( const GsMatn &a, const GsMatn &b, GsMatn &x )
 {
   GsMatn lu(a);
   x = b;
   return lusolve ( lu, x );
 }

bool inverse ( GsMatn &a, GsMatn &inva )
 {
   int j, k, n = a.lin();
   inva.size(n,n);
   double* buf=0;

   DBUFF ( buf, n ); // use static buffer
   GsMatn b; b.adopt(*DBuff,n,1);

   const int *indx = ludcmp ( a );
   if ( !indx ) return false;

   for ( j=0; j<n; j++ )
	{ for ( k=0; k<n; k++ ) buf[k]=0.0; // buf and b point to the same data
	  buf[j]=1.0;
	  lubksb ( a, b, indx );
	  for ( k=0; k<n; k++ ) inva(k,j)=b[k]; // same as: inva.set_column(j,b,0);
	}
   
   b.abandon ( *DBuff );
   return true;
 }

bool invert ( GsMatn &a )
 {
   GsMatn inva;
   if ( !inverse(a,inva) ) return false;
   a.adopt(inva);
   return true;
 }

double det ( GsMatn &a )
 {
   int n = a.lin();
   double d;
   ludcmp ( a, &d );
   for ( int i=0; i<n; i++ ) d *= a(i,i);
   return d;
 }

/* adapted from num recipes code (but not yet ok) : */
/*
bool gauss2 ( GsMatn &a, GsMatn &b, GsMatn &x )
 {
   int n = a.lin();
   int m = b.col();
   int i, icol, irow, j, k, l, ll;
   double big, pivinv, tmp;

   static int *indxc=0;
   static int *indxr=0;
   static int *ipiv=0;
   static int cur_size=0;
   if ( cur_size<n ) 
	{ delete[] indxc; indxc=new int[n]; 
	  delete[] indxr; indxr=new int[n]; 
	  delete[] ipiv;  ipiv=new int[n]; 
	  cur_size=n; 
	}

   for ( j=0; j<n; j++ ) ipiv[j]=0;

   for ( i=0; i<n; i++ )
	{ big=0.0;
	  for ( j=0; j<n; j++ )
	   { if ( ipiv[j]==0 ) continue;
		 for ( k=0; k<n; k++ )
		  { if ( ipiv[k]==0 ) 
			 { tmp=GS_ABS(a(j,k));
			   if ( tmp>=big) { big=tmp; irow=j; icol=k; }
			 } 
			else if ( ipiv[k]>0 ) 
			 { gsout.warning("gaussj: Singular Matrix-1"); return false; }
		  }
		 ++(ipiv[icol]);
		 if ( irow!=icol )
		  { for ( l=0; l<n; l++ ) GS_SWAP( a(irow,l), a(icol,l) )
			for ( l=0; l<m; l++ ) GS_SWAP( b(irow,l), b(icol,l) )
		  }

		 indxr[i]=irow;
		 indxc[i]=icol;
		 if ( a(icol,icol)==0.0 ) { gsout.warning("gaussj: Singular Matrix-2"); return false; }

		 pivinv = 1.0/a(icol,icol);
		 a(icol,icol)=1.0;

		 for ( l=0; l<n; l++ ) a(icol,l) *= pivinv;
		 for ( l=0; l<m; l++ ) b(icol,l) *= pivinv;

		 for ( ll=0; ll<n; ll++ )
		  { if ( ll==icol) continue;
			tmp=a(ll,icol);
			a(ll,icol)=0.0;
			for ( l=0; l<n; l++ ) a(ll,l) -= a(icol,l)*tmp;
			for ( l=0; l<m; l++ ) b(ll,l) -= b(icol,l)*tmp;
		  }
	   }
	  for ( l=n-1; l>=0; l-- ) 
	   { if ( indxr[l]==indxc[l] ) continue;
		 for ( k=0; k<n; k++ )
		   GS_SWAP ( a(k,indxr[l]), a(k,indxc[l]) );
	   }
	}
   return true;
 }
*/

// my gauss implementation :
bool gauss ( const GsMatn &a, const GsMatn &b, GsMatn &x )
 {
   int i, j, k, n, piv_lin;
   double tmp, piv_val;
   static GsMatn m;

   n = a.lin();
   x.size ( n, 1 );
   m.size ( n, n+1 );
   m.submat ( 0, 0, a, 0, n-1, 0, n-1 );
   m.column ( n, b, 0 );

   for ( i=0; i<n; i++ ) // loop lines
	{
	  piv_lin=i; piv_val=GS_ABS(m(i,i));
	  for ( k=i+1; k<n; k++ ) // loop lines below i
		{ tmp=GS_ABS(m(k,i));
		  if ( tmp>piv_val ) { piv_lin=k; piv_val=tmp; }
		}
	  if ( i!=piv_lin ) m.swaplines(i,piv_lin);

	  tmp = m(i,i);
	  if ( tmp==0.0 ) { gsout.warning("singular matrix in gauss\n"); return false; }
	  for ( k=i+1; k<n; k++ )
		for ( j=n; j>=i; j-- )
		   m(k,j) = m(k,j)-m(i,j)*m(k,i)/tmp;
	}

   for ( i=n-1; i>=0; i-- )
	{ tmp = 0.0;
	  for ( k=i+1; k<n; k++ ) tmp += m(i,k)*x[k];
	  if ( m(i,i)==0.0 ) { gsout.warning("singular matrix in gauss (2)\n"); return false; }
	  x[i] = (m(i,n)-tmp) / m(i,i);
	}

   return true;
 }

/* Adapted from num. rec., it works fine. a,b,c are the vectors of each column
of the tridiagonal matrix m, to find u such that m*u=r. n is the number of lines of m
void tridag ( float* a, float* b, float* c, float* r, float* u, int n )
 {
   int j;
   float bet;

   static int s=0;
   static float* gam=0;

   if ( s<n ) { s=n; gam=(float*)realloc(gam,s*sizeof(float)); }

   if ( b[0]==0 ) gsout.warning("Error 1 in tridag");

   u[0]=r[0]/(bet=b[0]);

   for ( j=1; j<n; j++ )
	{ gam[j] = c[j-1]/bet;
	  bet = b[j]-a[j]*gam[j];
	  if ( bet==0.0 ) gsout.warning("Error 2 in tridag");
	  u[j] = (r[j]-a[j]*u[j-1])/bet;
	}

   for ( j=(n-2); j>=0; j-- ) u[j] -= gam[j+1]*u[j+1];
 }
*/

/* From my old libraries, should work.
static void gauss_band ( GsMatn& A, GsMatn& B )
 {
   int i, j, k, c;
   int band=(A.col()-2)/2;
   int diag=band+1;	   // diagonal
   int last=col-1;	// last column of band TMatrix
   real fact, div;

   if (col%2!=0 || col>lin || pointer==null) return gaBadSize;
   if ( !v.setSize(lin,1) ) return gaNoMemory;

   for ( i=1; i<=lin; i++ )
	{ c=col;
	  div = MAT(i,band+1);
	  if ( div==0.0 ) return gaZeroDivision;
	  for ( j=i-band; j<=i+band && j<=lin; j++ )
	   { c--;
		 if (j<=i || j<1) continue;
		 fact = MAT(j,c)/div;
		 MAT(j,col) = MAT(j,col)-MAT(i,col)*fact;
		 for ( k=last; k>=1; k-- )
		   {  if (k+j-i>last) continue;
			  MAT(j,k) = MAT(j,k)-MAT(i,k+j-i)*fact;
		   }
	   }
	}
   for ( j=lin; j>=1; j-- )
	{
	  fact = 0.0; c=diag;
	  for ( k=j+1; k<=j+band && k<=lin; k++ )
		{ ++c; fact += MAT(j,c)*v.get(k); }
	  if ( MAT(j,diag)==0.0 ) return gaZeroDivision;
	  v(j) = ( (MAT(j,col)-fact) / MAT(j,diag) );
	}
   return gaOk;
 }
*/

//================================= End Of File ==================================
