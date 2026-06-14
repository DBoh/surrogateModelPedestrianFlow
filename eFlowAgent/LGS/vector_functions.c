/*!
  \file vector_functions.c
  \brief functions for vector operations
*/
#include <ped.h>

extern REAL GridEpsZero;

int Colinear(REAL a1[2], REAL a2[2], REAL b1[2], REAL b2[2]);
int SameLine(REAL a1[2], REAL a2[2], REAL b1[2], REAL b2[2]);

/*!
  \brief scalar product

  \f[<u,v>=\sum_{i=0}^{N-1}u_iv_i\f]
*/
REAL scalpr(REAL *u, REAL *v, int N)
{
  int i;
  REAL out=0.;

  for(i=0 ; i<N ; i++)
    out += u[i]*v[i];

  return out;
}

/*!
  \brief cross produkt

  \f[u\times v = u_1v_2-u_2v_1\f]

*/
REAL CrossProd_2d(REAL *u, REAL *v){return u[0]*v[1]-v[0]*u[1];}

/*!
  \brief looks for intersection between two vectors

  interpreted as sets of points.

  \f[c=a\cap b\f]

  where \f$a=(a_1,a_2)\f$, \f$b=(b_1,b_2)\f$, \f$c=(c_1,c_2)\f$ and \f$c\f$ has the direktion of \f$b\f$.

*/
int VecCapVec(REAL a1[2], REAL a2[2], REAL b1[2], REAL b2[2], REAL c1[2], REAL c2[2])
{
  int a1b, a2b, b1a, b2a, l, sit=0;
  REAL a[2], b[2], ab, la=0., lb=0., lab=0.;

  for(l=0 ; l<2 ; l++){
    c1[l] = 0.;
    c2[l] = 0.;
  }

  if(!Colinear(a1,a2,b1,b2)) return 0;
  else if(!SameLine(a1,a2,b1,b2)) return 0;


  for(l=0 ; l<2 ; l++){
    a[l]  = a2[l]-a1[l];
    b[l]  = b2[l]-b1[l];
    la   += pow(a[l],2.);
    lb   += pow(b[l],2.);
  }

  la = sqrt(la);
  lb = sqrt(lb);
  ab = scalpr(a,b,2);

  if(ab<0)
    for(l=0 ; l<2 ; l++){
      ab    = a2[l];
      a2[l] = a1[l];
      a1[l] = ab;
    }

  a1b = PointElementVec(a1,b1,b2);
  a2b = PointElementVec(a2,b1,b2);
  b1a = PointElementVec(b1,a1,a2);
  b2a = PointElementVec(b2,a1,a2);

  sit = a1b+2*a2b+4*b1a+8*b2a;


  switch(sit){
  case 3:
  case 11:
    for(l=0 ; l<2 ; l++){
      c1[l] = a1[l];
      c2[l] = a2[l];
    }
    break;
  case 12:
  case 14:
    for(l=0 ; l<2 ; l++){
      c1[l] = b1[l];
      c2[l] = b2[l];
    }
    break;
  case 6:
    lab  = pow(b2[0]-a1[0],2.);
    lab += pow(b2[1]-a1[1],2.);
    lab  = sqrt(lab);
    if(fabs(la+lb-lab)<GridEpsZero){
      sit = 0;
      break;
    }
  case 7:
    for(l=0 ; l<2 ; l++){
      c1[l] = b1[l];
      c2[l] = a2[l];
    }
    break;
  case 9:
    lab  = pow(a2[0]-b1[0],2.);
    lab += pow(a2[1]-b1[1],2.);
    lab  = sqrt(lab);
    if(fabs(la+lb-lab)<GridEpsZero){
      sit = 0;
      break;
    }
  case 13:
  case 15:
    for(l=0 ; l<2 ; l++){
      c1[l] = a1[l];
      c2[l] = b2[l];
    }
    break;
  default:
    sit=0;
    break;
  }

/*   if(sit) */
/*     printf("Situation: %d %d %d %d [%d]\n",a1b,a2b,b1a,b2a,sit); */

  return sit;
}


/*!
  \brief Check if \f$a\in \vec{b}=(b_1,b_2)\f$

*/
int PointElementVec(REAL a[2], REAL b1[2], REAL b2[2])
{
  int l, out=0;
  REAL aa[2], b[2], nb, sb, s;

  for(l=0 ; l<2 ; l++){
    b[l]  = b2[l]-b1[l];
    aa[l] = a[l]-b1[l];
  }

  nb = scalpr(b,b,2);         // nb = ||b||^2
  sb = scalpr(aa,b,2);         // sb = <a-b1,b>

  if(nb>GridEpsZero){
    s = sb/nb;
    if(s>-GridEpsZero && s<1.+GridEpsZero)
      out = 1;
#if WMSG
  } else {

    ErrorMessage("b has length zero","PointElementVec");
#endif
  }

  return out;
}


int Colinear(REAL a1[2], REAL a2[2], REAL b1[2], REAL b2[2])
{
  int l;
  REAL a[2], b[2], s1, s2;

  for(l=0 ; l<2 ; l++){
    a[l] = a2[l]-a1[l];
    b[l] = b2[l]-b1[l];
  }

  // Nullfaelle abschoepfen
  if(fabs(b[0])<GridEpsZero && fabs(a[0])>GridEpsZero) return 0;
  if(fabs(b[0])>GridEpsZero && fabs(a[0])<GridEpsZero) return 0;

  if(fabs(b[1])<GridEpsZero && fabs(a[1])>GridEpsZero) return 0;
  if(fabs(b[1])>GridEpsZero && fabs(a[1])<GridEpsZero) return 0;

  if(fabs(b[0])<GridEpsZero && fabs(a[0])<GridEpsZero) return 1;
  if(fabs(b[1])<GridEpsZero && fabs(a[1])<GridEpsZero) return 1;

  s1 = a[0]/b[0]; s2 = a[1]/b[1];

  if(fabs(s1-s2)<GridEpsZero) return 1;
  else return 0;
}


int SameLine(REAL a1[2], REAL a2[2], REAL b1[2], REAL b2[2])
{
  int l;
  REAL s1, s2;

  for(l=0 ; l<2 ; l++){
    if(fabs(b2[l]-b1[l])<GridEpsZero && fabs(a1[l]-b1[l])>=GridEpsZero)
      return 0;
    if(fabs(b2[l]-b1[l])<GridEpsZero && fabs(a1[l]-b1[l])<GridEpsZero)
      return 1;
  }

  s1 = (a1[0]-b1[0])/(b2[0]-b1[0]);
  s2 = (a1[1]-b1[1])/(b2[1]-b1[1]);

  if(fabs(s1-s2)<GridEpsZero) return 1;
  else return 0;
}

/*!
  \brief Hadamard Multiplication
  
  component by component multiplication according to
  \f[(VW)_i = \alpha\,V_iW_i\f]
*/
void HadamardMult(REAL *VW, REAL *V, REAL *W, REAL alpha, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    VW[i] = alpha*V[i]*W[i];

}


/*!
  \brief Hadamard Divide
  \detail component by component multiplication according to
  \((VW)_i = \alpha\,V_i/W_i\)
*/
void HadamardDiv(REAL *VW, REAL *V, REAL *W, REAL alpha, int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    VW[i] = alpha*V[i]/W[i];

}
