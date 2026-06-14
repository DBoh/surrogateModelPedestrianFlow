/*!
  \file ped_functions.c
  \brief functions for pedestrian flow simulation (ped)
*/
#include <ped.h>

extern int NT, NK, bandwidth;
extern int *nach;
extern int QuadM, QuadN;
extern REAL EPS;
extern REAL EpsZero;
extern REAL *XY;
extern DAT PED;
extern GE *tri;
extern GN *node;
extern TRI refine;
extern MATRIX MAT;
extern VIDEO video;
extern QUADRA *quad;
extern SUBDOM SDCV, RhoIni, Att;
extern NEUTRALS ZO;
extern POLY     DomainPoly, *Dpoly;
extern ShortestPath *SP;
extern int EikonalMode;
extern Monitoring MONI;

/*!
  \brief total mass in Omega

  \f[\int\limits_\Omega\varrho~dx\f]
 */
REAL TotalMass(REAL *M, REAL *rho)
{
  REAL TM;

  TM = VecVecIntegral(M,rho,ZO.NKones)*PED.CP*PED.CL*PED.CL;

  return TM;
}

int PED_exit(int ei)
{
  int out=0;

  if(ei%2!=0 && ei<-2) out=1;
  return out;

}


int PED_entree(int ei)
{
  int out=0;

  if(ei%2==0 && ei<-2) out=1;
  return out;

}


void FreeParam()
{
  int l, i, j;

  free(MAT.Mass);
  free(MAT.Stiff);
  free(MAT.MassExit);
  free(MAT.KonvExit);

  free(PED.EntreePersons);
  free(PED.MaxEntreePersons);
  for(l=0 ; l<4 ; l++)
    free(PED.entree[l]);
  
  if(PED.numentree){
    free(MAT.MassEntree);
    free(MAT.KonvEntree);
    free(PED.pFlowDoor);
    free(PED.DensityDoor);
    free(PED.MaxTimeDoor);
  }		  
  
  free(XY);

  for(i=0 ; i<=NK ; i++){
    free(node[i].NEIGH_NODE);
    free(node[i].NEIGH_TRI);
  }

  free(node);
  free(nach);
  free(tri);

  free(PED.ExitAttraction);

  for(l=0 ; l<4 ; l++){
    free(PED.exit[l]);
    free(PED.MSx[l]);
    free(refine.dOmega[l]);
  }

  free(refine.dOmegaList);
  free(refine.dOmegaStartlist);

  free(MONI.MSpeople);
  free(MONI.Exits);
  
  for(l=0 ; l<2 ; l++){
    free(PED.FundamentalDiagram[l]);
    free(PED.dFundamentalDiagram[l]);
  }

  free(PED.ARMask);

  for(l=0 ; l<7 ; l++)
    free(PED.ASx[l]);
  free(PED.ASphi);
  

  for(l=0 ; l<QuadM*QuadN ; l++)
    free(quad[l].Q);
  
  free(quad);
  
  //  free(RhoIni.P[0].XY);

  for(l=0 ; l<RhoIni.NumSubdom ; l++)
    free(RhoIni.P[l].XY);

  free(RhoIni.KnotToSubdom);
  free(RhoIni.Kvec);
  free(RhoIni.Tvec);
  free(RhoIni.P); 

  free(PED.CVsd);
  free(SDCV.KnotToSubdom);
  free(SDCV.Kvec);
  for(l=0 ; l<SDCV.NumSubdom ; l++)
    free(SDCV.P[l].XY);
  free(SDCV.P);
  
  free(Att.KnotToSubdom);
  free(Att.Kvec);
  free(Att.Tvec);

  sprintf(refine.subdomainRhoini,"%c",'\0');
  sprintf(refine.subdomainFD,"%c",'\0');

  if(DomainPoly.NumK)
    {
      free(DomainPoly.XY);
      free(DomainPoly.SEG);
      if(DomainPoly.NumH)
	free(DomainPoly.HOLES);
      free(DomainPoly.PolyNo);
      for(l=0 ; l<DomainPoly.NumH+1 ; l++)
	{
	  free(Dpoly[l].SEG);
	  free(Dpoly[l].XY);
	}
      free(Dpoly);
    }
  
#if TP

    for(l=0 ; l<2 ; l++)
      free(video.TrackXYPosition[l]);
    
    free(video.TrackInside);
    free(video.Rhoh);
    free(video.carry);
    free(video.pinloc);
    free(video.pinsum);
    free(video.pinflag);
#endif

#if SIRA
    free(video.SIRmode);
#endif

    free(ZO.NKones);
    free(ZO.NKzeros);
    free(ZO.NTones);
    free(ZO.NTzeros);
    free(ZO.Entreeones);
    free(ZO.Exitones);

    if(EikonalMode==2)
      {
	for(l=0 ; l<NT ; l++)
	  free(SP[l].Geom);
	free(SP);
      }
}

REAL DistPointdOmega(REAL x, REAL y,int *kk, int *ll)
{
  int i, k, l, i1, i2, kmin, lmin;
  REAL distP, mindist=1;

  for(i=0 ; i<refine.numdOmega ; i++){
    k = refine.dOmega[0][i]; l = refine.dOmega[1][i];
    i1 = tri[k].VNR[(l+1)%3];
    i2 = tri[k].VNR[(l+2)%3];
    distP = DistPointLineSegment(x,y,XY[2*i1],XY[2*i1+1],XY[2*i2],XY[2*i2+1]);
    if(distP<=mindist){
      mindist = distP; kmin = k; lmin = l;
    }
  }

  *kk = kmin; *ll = lmin;
  return mindist;


}
