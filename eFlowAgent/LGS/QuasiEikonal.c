/*!
  \file QuasiEikonal.c
  \brief an alternative for the eikonal equation
*/
#include <ped.h>

extern REAL EpsZero, EpsReg;
extern GN *node;
extern int NT, NK;
extern GE *tri;
extern DAT PED;
extern ShortestPath *SP;

void SetLevel(int k, int *Level);


REAL RhoWeight(REAL *r, int k)
{
  int l;
  REAL rm=0.;

  for(l=0 ; l<3 ; l++)
    rm += r[tri[k].VNR[l]];

  return rm/3.;
}

void ExitDist(int k, REAL *rho, REAL *Flag, REAL edist)
{
  int l, kl;
  REAL dist;

  for(l=0 ; l<3 ; l++)
    {
      kl = tri[k].NEIGH[l];
      if(Flag[kl]<0 || Flag[kl]>edist)
	{
	  Flag[kl] = edist;
	  dist = gridwidth_loc(kl); // area(kl);
	  dist += RhoWeight(rho,kl);
	  ExitDist(kl,rho,Flag,edist+dist);
	}
    }
}

void QuasiEikonalEquation(int LOOP, REAL *rho, REAL *phi)
{
  int k, l, ll, count,ne;
  int *NodeCount, *StartTri;
  REAL *TriDistFlag, dist;

  TriDistFlag = (REAL *)calloc(NT , sizeof(REAL ));
  StartTri = (int *)calloc(NT , sizeof(int ));
  NodeCount = (int *)calloc(NK , sizeof(int ));
  
  FieldCp(TriDistFlag,0.,TriDistFlag,-1.,NT);
  FieldCp(phi,0.,phi,0.,NK);
  
  for(k=0 ; k<NT ; k++)
    {
      count = 0;
      for(l=0 ; l<3 ; l++)
	{
	  ll = tri[k].VNR[l];
	  if(ChkExit(ll))
	    {
	      ne=ChkExit(ll);
	      count++;
	    }
	  NodeCount[ll]++;
	}
      if(count>=2)
	{
	  TriDistFlag[k] = max_REAL(0.,1-PED.ExitAttraction[ne-1]);
	  StartTri[k] = 1;
	}
    }


  for(k=0 ; k<NT ; k++)
    {
      if(StartTri[k])
	{
	  dist = gridwidth(k);
	  dist += RhoWeight(rho,k);
	  ExitDist(k,rho,TriDistFlag,dist);
	}
    }

  
  for(k=0 ; k<NT ; k++)
    {
      for(l=0 ; l<3 ; l++)
	{
	  ll = tri[k].VNR[l];
	  phi[ll] += TriDistFlag[k]/NodeCount[ll];
	}
    }

  
#if PLOT==2
  octaveplot(phi,"QEphi",0.,1.,LOOP);
  PlotBDphi(LOOP,phi,rho,0);
#endif
  

  free(TriDistFlag);
  free(NodeCount);
  free(StartTri);
}



void QuasiEikonalGeom()
{
  int *Level, k, j, MaxLevel, ml, i, k0, l, kk;


  Level = (int *)calloc(NT , sizeof(int ));
  SP = (ShortestPath *)malloc(NT * sizeof(ShortestPath ));
  
  FieldCpInt(Level,0,Level,-1,NT);

  for(k=0 ; k<NT ; k++)
    for(j=0 ; j<3 ; j++)
      if((tri[k].NEIGH[j]<0 && ChkExit(tri[k].VNR[(j+2)%3])) ||
	 (tri[k].NEIGH[j]<0 && ChkExit(tri[k].VNR[(j+1)%3])) )
	{
	  Level[k] = 0;
	  SP[k].mom=-1;
	}
  
  for(k=0 ; k<NT ; k++)
    if(Level[k]==0)
      SetLevel(k,Level);


  for(k=0 ; k<NT ; k++)
    {
      SP[k].Geom = (int *)calloc( Level[k]+1 , sizeof(int ));
      k0 = k; kk = k;
      for(l=Level[k] ; l>=0 ; l--)
	{
	  SP[k0].Geom[Level[kk]] = kk;
	  kk = SP[kk].mom;
	}
      SP[k].Level=Level[k];
    }


#if PLOT==2
  
  REAL *Dist;
  int *iCount;
  REAL fac=sqrt(3)/2, sum;

  Dist  = (REAL *)calloc(NK , sizeof(REAL ));
  iCount = (int *)calloc(NK , sizeof(int ));

  
  for(k=0 ; k<NT ; k++)
    {
      sum = 0;
      for(l=0 ; l<=Level[k] ; l++)
	{
	  kk = SP[k].Geom[l];
	  sum += gridwidth_loc(kk)*fac;
	}
      
      for(j=0 ; j<3 ; j++)
	{
	  Dist[tri[k].VNR[j]] += sum;
	  iCount[tri[k].VNR[j]]++;
	}
    }
  
  for(i=0 ; i<NK ; i++)
    Dist[i] /= iCount[i];
  
  octaveplot(Dist,"Dist",0.,-1.,0);

  free(Dist);
  free(iCount);
#endif
  
  free(Level);

}


void QuasiEikonalSP(int LOOP, REAL *rho, REAL *phi)
{
  int k, kk, l, j, i;
  int *iCount;
  REAL fac=sqrt(3)/2, sum, rw, frw, *f;

  f = (REAL *)calloc(NK , sizeof(REAL ));
  iCount = (int *)calloc(NK , sizeof(int ));
  FieldCp(phi,0.,phi,0.,NK);
  FD_Vec(f,rho,0);

  for(k=0 ; k<NT ; k++)
    {
      sum = 0;
      for(l=0 ; l<=SP[k].Level ; l++)
	{
	  kk = SP[k].Geom[l];

	  
	  rw = RhoWeight(rho,kk);
	  frw = FD_Value(rw,0,0.,0.,0);
	  frw = 1/sqrt(frw*frw+EpsReg*EpsReg);
	  
	  
	  //frw = (f[tri[k].VNR[0]]+f[tri[k].VNR[1]]+f[tri[k].VNR[2]])/3.;
	  sum += fac*gridwidth_loc(kk)*frw;
	}
      
      for(j=0 ; j<3 ; j++)
	{
	  phi[tri[k].VNR[j]] += sum;
	  iCount[tri[k].VNR[j]]++;
	}
    }
  
  for(i=0 ; i<NK ; i++)
    phi[i] /= iCount[i];
  
#if PLOT==2  
  octaveplot(phi,"SPphi",0.,-1.,LOOP);
  PlotBDphi(LOOP,phi,rho,0);
#endif

  free(iCount);
  free(f);

}



void SetLevel(int k, int *Level)
{
  int j, kk;

  for(j=0 ; j<3 ; j++)
    {
      kk = tri[k].NEIGH[j];
      if(kk>=0)
	{
	  if(Level[kk]<0 || Level[kk]>Level[k]+1)
	    {
	      Level[kk] = Level[k]+1;
	      SetLevel(kk,Level);
	      SP[k].child = kk;
	      SP[kk].mom = k;
	    }
	}
    }

}

/*
  flag = 1: phi=v
*/
void PlotBDphi(int LOOP, REAL *phi, REAL *rho, int flag)
{
  int l;
  REAL *dphi[2], *bdphi, *f;

  for(l=0 ; l<2 ; l++)
    dphi[l] = (REAL *)calloc(NK , sizeof(REAL *));
  bdphi = (REAL *)calloc(NK , sizeof(REAL *));
  f = (REAL *)calloc(NK , sizeof(REAL *));

  FD_Vec(f,rho,0);
  
  duh_vertex(phi,dphi);
  for(l=0 ; l<NK ; l++)
    {
      bdphi[l] = sqrt(dphi[0][l]*dphi[0][l]+dphi[1][l]*dphi[1][l]);
      f[l] = 1./sqrt(f[l]*f[l]+EpsZero*EpsZero);
    }
  
  octaveplot(bdphi,"Dphi",0.,-1.,LOOP);
  octaveplot(f,"Erhs",0.,-1.,LOOP);

  for(l=0 ; l<2 ; l++)
    free(dphi[l]);
  free(bdphi);
  free(f);
}
