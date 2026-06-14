/*!
  \file ped_monitoring.c
  \brief counting people
*/
#include <ped.h>

extern DAT PED;
extern System lgs;
extern GE *tri;
extern int  NK, bandwidth;
extern GN   *node;
extern REAL *XY, EpsReg;
extern DAT PED;
extern REAL GridEpsZero, EpsZero;
extern TRI refine;
extern MATRIX MAT;
extern SIRagent siragent;
extern NEUTRALS ZO;
extern Monitoring MONI;

void DiffusionVelocity(REAL *divelnu, REAL *rho);

int checkstep(REAL x1, REAL y1, REAL x2, REAL y2)
{
  int out=0;

  if(sqrt(pow(x1-x2,2.)+pow(y1-y2,2.)))
    out=1;

  return out;
}


/*!
  \brief Personenz&auml;hler*in 8-D
*/
void ped_monitoring(REAL *rho, REAL *vel[2], REAL *unu, int loop)
{
  int i;
  char buf[100];
  
  monitoring(rho,vel,unu,loop);

#if PLOT
  FILE *fp;

  if(!loop)
    {
      fp = fopen("00Output/Monitoring.dat","w");
      fprintf(fp,"t mass entree ");
      for(i=0 ; i<PED.numexit ; i++)
	{
	  sprintf(buf,"exit_%d",i);
	  fprintf(fp,"%s ",buf);
	}
      for(i=0 ; i<PED.MS ; i++)
	{
	  sprintf(buf,"MS_%d",i);
	  fprintf(fp,"%s ",buf);
	}
      fprintf(fp,"rhomin rhomax s e i r \n");
      /*
      for(i=0 ; i<MONI.MS ; i++)
	{
	  sprintf(buf,"MS%d",i);
	  fprintf(fp,"%s ",buf);
	}
      fprintf(fp,"\n"); 
      */
    }
  else
    fp = fopen("00Output/Monitoring.dat","a");

  
  fprintf(fp,"%.12e %.12e %.12e ",loop*PED.delta,MONI.Mass,-MONI.Entree);

  for(i=0 ; i<PED.numexit ; i++)
    {
      fprintf(fp,"%.12e ",MONI.Exits[i]);
    }

  for(i=0 ; i<MONI.MS ; i++)
    fprintf(fp,"%.12e ",MONI.MSpeople[i]);

  fprintf(fp,"%.12e %.12e %.12e %.12e %.12e %.12e ",MONI.Rhomin,MONI.Rhomax,MONI.S_EIR,MONI.S_E_IR,MONI.SE_I_R, MONI.SEI_R);


  fprintf(fp,"\n");
  
  fclose(fp);


#endif

}


void monitoring(REAL *rho, REAL *vel[2], REAL *unu, int loop)
{
  int l, ll;
  REAL *MA, vfak, dfak;
  REAL Seg[4], SUM;

  MA      = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));

  MONI.Mass = VecVecIntegral(MAT.Mass,rho,ZO.NKones)*PED.CP*PED.CL*PED.CL;

  //====================== AUSGANG ==========================================

  vfak = PED.delta*PED.CL*PED.CP*PED.CV/PED.CT;
  dfak = PED.delta*PED.EPS0*PED.CP;
  dfak = 0.;

  BoundIntegral(MA,unu,PED.numexit,PED.exit);
  MONI.Exit  = vfak*VecVecIntegral(MA,rho,ZO.Exitones);
  MONI.Exit -= -dfak*VecVecIntegral(MAT.KonvExit,rho,ZO.Exitones);

  SUM=0.;
  for(l=0 ; l<PED.numexit ; l++)
    {
      for(ll=0 ; ll<4 ; ll++)
	Seg[ll] = PED.exit[ll][l];
      BoundSegIntegral(MA,unu,Seg);
      MONI.Exits[l]  = vfak*VecVecIntegral(MA,rho,ZO.Exitones);
      MONI.Exits[l] -= -dfak*VecVecIntegral(MAT.KonvExit,rho,ZO.Exitones);
      SUM += MONI.Exits[l];
    }

#if WMSG
  if(fabs(SUM-MONI.Exit)>=EpsZero)
    Message("Wrong cumsum Exits",'.',"monitoring");
#endif
  
  //====================== EINGANG ==========================================

  if(PED.numentree){
    BoundIntegral(MA,unu,PED.numentree,PED.entree);
    MONI.Entree  = vfak*VecVecIntegral(MA,rho,ZO.Entreeones);
    MONI.Entree -= dfak*VecVecIntegral(MAT.KonvEntree,rho,ZO.Entreeones);
  }
  
  //====================== RhoMinMax ========================================
  // !!! hier noch Koordinaten dazu
  
  MONI.Rhomin = FieldMin(rho,NK);
  MONI.Rhomax = FieldMax(rho,NK);

  if (MONI.Mass > PED.MassMax) { PED.MassMax = MONI.Mass; }

  // ===================== SEIR =============================================

  MONI.S_EIR  = siragent.SEIRMonitoring[0];
  MONI.S_E_IR = siragent.SEIRMonitoring[1];
  MONI.SE_I_R = siragent.SEIRMonitoring[2];
  MONI.SEI_R  = siragent.SEIRMonitoring[3];
  
  //====================== Messstationen ====================================
  // !!! hier noch Rho-Kurve ueber MS als Graph

  for(l=0 ; l<MONI.MS ; l++)
    MONI.MSpeople[l] = -QuadSegment(1,PED.MSx[0][l],PED.MSx[1][l],PED.MSx[2][l],PED.MSx[3][l],rho,vel,loop)*vfak;


  


  free(MA);
}

void DiffusionVelocity(REAL *divelnu, REAL *rho)
{
  int l, i;
  REAL *drho[2];

  for(l=0 ; l<2 ; l++)
    drho[l] = (REAL *)calloc(NK , sizeof(REAL ));
  
  duh_vertex(rho,drho);

  for(i=0 ; i<NK ; i++)
    for(l=0 ; l<2 ; l++){
      drho[l][i] /= sqrt(pow(rho[i],2.)+pow(EpsReg,2.));
      drho[l][i] *= -PED.EPS0;
    }
  
  u_nu(divelnu,drho);
    
  for(l=0 ; l<2 ; l++)
    free(drho[l]);
}






