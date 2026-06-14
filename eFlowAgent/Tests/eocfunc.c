/*!
  \file eocfunc.c
  \brief funcitons for EOC-verifications
*/

#include <ped.h>

extern DAT PED;
extern REAL PI, *XY;
extern int NK;

REAL RhoEx(REAL x, REAL y, int ix, int iy)
{
  REAL Rho1, Rho2, d1Rho1, d1Rho2, d2Rho1, d2Rho2;

  Rho1 = 0.;
  Rho2 = 0.;

  d1Rho1 = 0.;
  d1Rho2 = 0.;

  d2Rho1 = 0.;
  d2Rho2 = 0.;

  if(ix==0 && iy==0) return Rho1*Rho2;
  if(ix==1 && iy==0) return d1Rho1*Rho2;
  if(ix==0 && iy==1) return Rho1*d1Rho2;
  if(ix==1 && iy==1) return d1Rho1*d1Rho2;
  if(ix==2 && iy==0) return d2Rho1*Rho2;
  if(ix==0 && iy==2) return Rho1*d2Rho2;

  ErrorMessage("something wrong with ix or iy","RhoEx");
  return 0;
}

REAL PhiEx(REAL x, REAL y, int ix, int iy)
{
  REAL Phi1, Phi2, d1Phi1, d1Phi2, d2Phi1, d2Phi2;

  Phi1 = sin(PI*(1-x)*0.5);
  Phi2 = cos(2*PI*y);

  d1Phi1 = -PI*0.5*cos(PI*(1-x)*0.5);
  d1Phi2 = -2*PI*sin(2*PI*y);

  d2Phi1 = -PI*PI*0.25*sin(PI*(1-x)*0.5);
  d2Phi2 = -4*PI*PI*cos(2*PI*y);




  if(ix==0 && iy==0) return Phi1*Phi2;
  if(ix==1 && iy==0) return d1Phi1*Phi2;
  if(ix==0 && iy==1) return Phi1*d1Phi2;
  if(ix==1 && iy==1) return d1Phi1*d1Phi2;
  if(ix==2 && iy==0) return d2Phi1*Phi2;
  if(ix==0 && iy==2) return Phi1*d2Phi2;

  ErrorMessage("something wrong with ix or iy","PhiEx");
  return 0;
}

REAL FDex(REAL x){ return 1-x; }

void RHS_Helm(REAL *rhs)
{
  int i;
  REAL vex, LaplacePhi, NabPhi, rho;

  for(i=0 ; i<NK ; i++)
    {
      rho = RhoEx(XY[2*i],XY[2*i+1],0,0);
      vex = exp(-1./PED.EPS0*PhiEx(XY[2*i],XY[2*i+1],0,0));
      LaplacePhi = PhiEx(XY[2*i],XY[2*i+1],2,0) + PhiEx(XY[2*i],XY[2*i+1],0,2);
      NabPhi = pow(PhiEx(XY[2*i],XY[2*i+1],1,0),2.)+pow(PhiEx(XY[2*i],XY[2*i+1],0,1),2.);
      rhs[i] = vex*(1./pow(FDex(rho),2.)+PED.EPS0*LaplacePhi-NabPhi);
    }

}
