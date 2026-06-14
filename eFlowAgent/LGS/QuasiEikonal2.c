/*!
  \file QuasiEikonal.c
  \brief an alternative for the eikonal equation
*/
#include <ped.h>

extern GN *node;
extern int NT, NK;
extern GE *tri;
extern DAT PED;
extern REAL EpsZero, EpsReg;
extern REAL *XY;

void FindPhi(int k, int l, REAL *rho, REAL *Phi, int rec);


void QuasiEikonal(REAL *rho, REAL *vel[2])
{
  REAL *Phi;
  int k, l, ll, l0, l1, l2, count, ne;
  int *StartTri;
  REAL *TriDistFlag, *NodeDist, dist, Phi2;
  //  REAL duh[2], phi_loc[3];
  
  Phi = (REAL *)calloc(NK , sizeof(REAL ));
  NodeDist = (REAL *)calloc(NK , sizeof(REAL ));
  TriDistFlag = (REAL *)calloc(NT , sizeof(REAL ));
  StartTri = (int *)calloc(NT , sizeof(int ));
  
  FieldCp(TriDistFlag,0.,TriDistFlag,-1.,NT);
  FieldCp(Phi,0.,Phi,-1.,NK);

  for(k=0 ; k<NT ; k++)
    {
      count = 0;
      for(l=0 ; l<3 ; l++)
	{
	  l0 = tri[k].VNR[l];
	  l1 = tri[k].VNR[(l+1)%3];
	  l2 = tri[k].VNR[(l+2)%3];
	  if(ChkExit(l0) && ChkExit(l1))
	    {
	      ne=ChkExit(l0);
	      TriDistFlag[k] = max_REAL(0.,1-PED.ExitAttraction[ne-1]);
	      //printf("Exit %d: %f \n",ne,TriDistFlag[k]);

	      Phi[l0] = 0;
	      Phi[l1] = 0;
	      FindPhi(k,l,rho,Phi,0);
	      
	      for(ll=0 ; ll<3 ; ll++)
		if(tri[k].NEIGH[ll]>=0)
		  StartTri[tri[k].NEIGH[ll]] = 1;
	      
	    }
	}
    }

  printf("Rekursion startet:\n");
  
  for(k=0 ; k<NT ; k++)
    if(StartTri[k])
      for(l=0 ; l<3 ; l++)
	if(Phi[tri[k].VNR[l]]>=0 && Phi[tri[k].VNR[(l+1)%3]]>=0 && Phi[tri[k].VNR[(l+2)%3]]<0)
	  FindPhi(k,l,rho,Phi,1); 
  

  octaveplot(Phi,"QPhi",0,1.,0);
  
  free(Phi);
  free(NodeDist);
  free(TriDistFlag);
  free(StartTri);

}


void FindPhi(int k, int l, REAL *rho, REAL *Phi, int rec)
{
  int kn, l0, l1, l2, ll;
  REAL x1d, x2d, Phid, r, a, b, c, a1, a2, b1, b2, c1, c2;
  REAL x0, y0, x1, y1, x2, y2, frho, Phi0, Phi1, Phi2, rhomean;
  REAL dummy1, dummy2, dummy3, dummy4;

  // Gesucht ist der Funktionswert Phi2, so das |grad Phi|=1/frho
  l0 = tri[k].VNR[l];
  l1 = tri[k].VNR[(l+1)%3];
  l2 = tri[k].VNR[(l+2)%3];

  x0=XY[2*l0]; y0=XY[2*l0+1];
  x1=XY[2*l1]; y1=XY[2*l1+1];
  x2=XY[2*l2]; y2=XY[2*l2+1];

  x1d = x0-x1;
  x2d = y0-y1;
  Phi0 = Phi[l0];
  Phi1 = Phi[l1];
  Phid = Phi0-Phi1;
  rhomean = (rho[l0]+rho[l1]+rho[l2])/3.;
  frho = FD_Value(rhomean,0,0.,0.,PED.FD);
  r = 1/frho/frho;

  if(fabs(x1d)<EpsZero)
    {

      b = Phid/x2d;
      if(r>=b*b)
	{
	  a1 = sqrt(r-b*b);
	  a2 = -a1;
	  c1 = Phi0-y0*b-x0*a1;
	  c2 = Phi0-y0*b-x0*a2;

	  a=a1; c=c1;
	  if(x2*a1+y2*b+c1<x2*a2+y2*b+c2)
	    {
	      a=a2; c=c2;
	    }

	    Phi2 = x2*a+y2*b+c;
	}        
      else
	{
	  a = 0;
	  b = 1./frho;
	  c = Phi0-b*y0;  
	  Phi2 = b*y2+c;
	}
    }
  else if(fabs(x2d)<EpsZero)
    {
      a = Phid/x1d;
      if(r>=a*a)
	{
	  b1 = sqrt(r-a*a);
	  b2 = -b1;
	  c1 = Phi1-y1*b-x1*a1;
	  c2 = Phi1-y1*b-x1*a2;

	  b=b1; c=c1;
	  if(x2*a+y2*b1+c1<x2*a+y2*b2+c2)
	    {
	      b=b2; c=c2;
	    }
	  
	  Phi2 = x2*a+y2*b+c;
	}
      else
	{
	  b=0;
	  a=1/frho;
	  c=Phi1-x1*a;
	  Phi2 = a*x2+c;
	}
    }
  else
    {

      dummy1 = x1d*x1d+x2d*x2d;
      dummy2 = Phid*x2d;
      dummy3 = Phid*Phid*x2d*x2d;
      dummy4 = Phid*Phid-x1d*x1d*r;

      if(dummy3-dummy1*dummy4>=0)
	{
	  b1 = (dummy2-sqrt(dummy3-dummy1*dummy4))/dummy1;
	  b2 = (dummy2+sqrt(dummy3-dummy1*dummy4))/dummy1;

	  a1 = b1*(y1-y0)/(x0-x1)+(Phi0-Phi1)/(x0-x1);
	  a2 = b2*(y1-y0)/(x0-x1)+(Phi0-Phi1)/(x0-x1);

	  c1 = Phi0-a1*x0-b1*y0;
	  c2 = Phi0-a2*x0-b2*y0;

	  a=a1; b=b1; c=c1;
	  if(x2*a1+y2*b1+c1<x2*a2+y2*b2+c2)
	    {
	      a=a2; b=b2; c=c2;
	    }
	  Phi2 = a*x2+b*y2+c;
	}
      else
	{
	  a = Phid*x2d/(x1d*x1d+x2d*x2d);
	  b = (Phid-x1d*a)/x2d;
	  c = Phi0-x0*a-y0*b;
	  Phi2 = a*x2+b*y2+c;
	}
    }

  printf("erledige T = %d, |grad Phi| = %f, 1/f = %f\n",k,sqrt(a*a+b*b),sqrt(r));

  Phi[tri[k].VNR[(l+2)%3]] = Phi2;

  if(rec)
    {
      // erster Nachar an neuer Ecke
      kn = tri[k].NEIGH[l];

      if(kn>=0)
	for(ll=0 ; ll<3 ; ll++)
	  if(Phi[tri[kn].VNR[ll]]>=0 && Phi[tri[kn].VNR[(ll+1)%3]]>=0 && Phi[tri[kn].VNR[(ll+2)%3]]<0)
	    FindPhi(kn,ll,rho,Phi,1); 
  
      // zweiter Nachbar an neuer Ecke
      kn = tri[k].NEIGH[(l+1)%3];

      if(kn>=0)
	for(ll=0 ; ll<3 ; ll++)
	  if(Phi[tri[kn].VNR[ll]]>=0 && Phi[tri[kn].VNR[(ll+1)%3]]>=0 && Phi[tri[kn].VNR[(ll+2)%3]]<0)
	    FindPhi(kn,ll,rho,Phi,1); 
    }
}
