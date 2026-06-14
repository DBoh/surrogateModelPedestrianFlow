/*!
  \file matrixtest.c
  \brief funcitons that checks matrizes
*/

#include <ped.h>

extern DAT PED;
extern REAL *XY;
extern int NK, bandwidth;
extern TRI refine;
extern MATRIX MAT;
extern REAL PI;


void MatrixTest(int flag)
{
  int i;
  REAL *v1, *v2, *v3, *v4, *v5[2];
  REAL *Mv, *Sv, *Kv, h=gridwidth();
  REAL Mint, Mvint, Sint, Svint, Kvint;
  REAL Mex, Mvex, Sex, Svex, Kvex;

  Mv  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  Sv  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  Kv  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  v1  = (REAL *)calloc(NK , sizeof(REAL ));
  v2  = (REAL *)calloc(NK , sizeof(REAL ));
  v3  = (REAL *)calloc(NK , sizeof(REAL ));
  v4  = (REAL *)calloc(NK , sizeof(REAL ));
  v5[0] = (REAL *)calloc(NK , sizeof(REAL ));
  v5[1] = (REAL *)calloc(NK , sizeof(REAL ));

  for(i= 0 ; i<NK ; i++){
    v1[i] = 10.*pow(XY[2*i],2.)*cos(XY[2*i+1]);
    v3[i] = 18.*pow(XY[2*i+1],2.)*sin(XY[2*i]);
    v4[i] = 14.*log(XY[2*i]*XY[2*i+1]+1); 
    v5[0][i] = exp(XY[2*i]);
    v5[1][i] = sqrt(XY[2*i+1]); 
  }

  SysMatrixV(Mv,v4,mass_elem_v);
  SysMatrixV(Sv,v4,stiff_elem_v);
  SysMatrixK(Kv,v5,konv_elem_v);

  Mex=1.550613767709267066;
  Mvex=5.542968064184253585;
  Sex=2.037762880030211665;
  Svex=3.503598459981268842;
  Kvex=8.558365379457976374; //8.766791702351818927;
  /*
  if(NK<=10)
    PrintSysMatrix("Matrix",Mv);
  */
  
  amul(MAT.Mass,v1,v2);
  Mint = scalpr(v2,v3,NK);
  amul(Mv,v1,v2);
  Mvint = scalpr(v2,v3,NK);
  amul(MAT.Stiff,v1,v2);
  Sint = scalpr(v2,v3,NK);
  amul(Sv,v1,v2);
  Svint = scalpr(v2,v3,NK);
  amul(Kv,v1,v2);
  Kvint = scalpr(v2,v3,NK);

  printf("%lf %e %e %e %e %e\n",h,fabs(Mex-Mint),fabs(Mvex-Mvint),fabs(Kvex-Kvint),fabs(Sex-Sint),fabs(Svex-Svint)); 

  if(flag) exit(0);
}

void BoundMatrixTest(int flag)
{
  int i;
  REAL *v1, *v2, *v3, *v5[2];
  REAL *MA, *ME, h=gridwidth();
  REAL MAint, MEint, MAex, MEex;

  MA  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  ME  = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  v1  = (REAL *)calloc(NK , sizeof(REAL ));
  v2  = (REAL *)calloc(NK , sizeof(REAL ));
  v3  = (REAL *)calloc(NK , sizeof(REAL ));
  v5[0] = (REAL *)calloc(NK , sizeof(REAL ));
  v5[1] = (REAL *)calloc(NK , sizeof(REAL ));

  for(i= 0 ; i<NK ; i++){
    v1[i] = 1./exp(XY[2*i]);
    v3[i] = pow(XY[2*i+1]+1,3.); 
    v5[0][i] = pow(XY[2*i+1]+1,2.); 
    v5[1][i] = 0.;
  }

  if(0){
  MEex = 818181./524288.; // + exp(-3./4.)*(exp(1./2.) - 1);

  printf("Entree: Num=%d\n",PED.numentree);
  BoundIntegral(ME,v5[0],PED.numentree,PED.entree);
  amul(ME,v1,v2);
  MEint = scalpr(v2,v3,NK);

  //  printf("%lf %e (%f)\n",h,fabs(MEex-MEint),MEint);

  MAex = -(63651.*exp(-1.))/125000.; // + (243.*exp(-4./5.)*(exp(2./5.) - 1.))/32.;
  
  printf("Exit: Num=%d\n",PED.numexit);
  BoundIntegral(MA,v5[0],PED.numexit,PED.exit);
  amul(MA,v1,v2);
  MAint = scalpr(v2,v3,NK);
  
  printf("%lf %e %e (%lf %lf)\n",h,fabs(MEex-MEint),fabs(MAex-MAint),MEint,MAint);
  }
  
  MEex = 818181./524288.; // + exp(-3./4.)*(exp(1./2.) - 1);

  printf("Entree: Num=%d\n",PED.numentree);
  BoundFlowIntegral(ME,v5[0],PED.numentree,PED.entree);
  amul(ME,v1,v2);
  MEint = scalpr(v2,v3,NK);

  MAex = -MEex/exp(1); //-(63651.*exp(-1.))/125000.; // + (243.*exp(-4./5.)*(exp(2./5.) - 1.))/32.;
  
  printf("Exit: Num=%d\n",PED.numexit);
  BoundFlowIntegral(MA,v5[0],PED.numexit,PED.exit);
  amul(MA,v1,v2);
  MAint = scalpr(v2,v3,NK);
  
  printf("%lf %e %e (%lf %lf)\n",h,fabs(MEex-MEint),fabs(MAex-MAint),MEint,MAint);
  if(flag) exit(0);
}

void VektorTest(REAL *v, int flag)
{
  REAL *v1, xrhs;
  v1 = (REAL *)calloc(NK , sizeof(REAL ));

  field_cp(v1,0.,v1,0.,v1,1.,NK);

  xrhs = scalpr(v,v1,NK);
  printf("xrhs = %f [%f]\n",xrhs,0.04+PED.delta/PED.CP*(0.1-0.2-0.2*PED.EPS0));

  if(flag) exit(0);
}
