/*!
  \file tools_fnctions.c
  \brief some unclassified routines
*/
#include <ped.h>

extern REAL EpsZero;
extern int NT, NK, bandwidth;
extern GE *tri;
extern REAL *XY;
extern POLY DomainPoly, *Dpoly;
extern DAT PED;
extern SUBDOM RhoIni;


void DistField(REAL *d, REAL ax, REAL ay, REAL bx, REAL by)
{
  int i;

  for(i=0 ; i<NK ; i++){
    d[i] = dist(XY[2*i],XY[2*i+1],ax,ay,bx,by);
  }

}

REAL CrossProduct_2d(REAL x[2], REAL y[2]){return x[0]*y[1]-x[1]*y[0];}

/*! \brief Abstand eines Punktes (x,y) zu einer Geraden durch die Punkte a,b */
REAL dist(REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by)
{
  REAL xp, yp;

  OrthogonalProjection(&xp,&yp,x,y,ax,ay,bx,by);
  return sqrt(pow(xp-x,2.)+pow(yp-y,2.));

}


/*! \brief Abstand eines Punktes (x,y) zu einer Strecke von a nach b*/
REAL DistSeg(REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by)
{
  REAL xp, yp, A[2], B[2], NA, NB;
  REAL out=-1;
  
  OrthogonalProjection(&xp,&yp,x,y,ax,ay,bx,by);

  if(PointElementSegment(xp,yp,ax,ay,bx,by)){
    out = sqrt(pow(xp-x,2.)+pow(yp-y,2.));
  } else {
    A[0] = ax-x; A[1] = ay-y; NA = norm(A,2,2.);
    B[0] = bx-x; B[1] = by-y; NB = norm(B,2,2.);
    if(NA<NB) out = NA; else out = NB;
  }
  return out;
}

/*! \brief ``Orthogonale'' Projektion auf eine Strecke */
void OrthogonalProjection(REAL *xp, REAL *yp, REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by)
{
  REAL v1[2], v2[2], sv, P[2], sp, dist1, dist2, dist3;
  REAL spz, spn;

  v1[0] = x-ax; v1[1] = y-ay;   // x-a
  v2[0] = bx-ax; v2[1] = by-ay; // b-a

  // P ist "orthogonale" Projektion von (x,y) auf die Strecke zwischen a und b
  spz   = scalpr(v1,v2,2);
  spn   = scalpr(v2,v2,2);
  sp = min_REAL(max_REAL(0.,spz/spn),1.);
  *xp = ax + sp*v2[0]; *yp = ay + sp*v2[1];

}

void FuncVec(REAL t, REAL *v, REAL (*f)(REAL , REAL , REAL ))
{
  int i;

  for(i=0 ; i<NK ; i++)
    v[i] = f(t,XY[2*i],XY[2*i+1]);
}


