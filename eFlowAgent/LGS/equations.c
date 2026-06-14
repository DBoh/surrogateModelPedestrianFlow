/*!
  \file equations.c
  \brief several equations
*/
#include <ped.h>

extern TRI refine;
extern SOLV solver;
extern REAL EpsZero, EpsReg;
extern DAT PED;
extern MATRIX MAT;
extern SUBDOM SDCV;
extern GN *node;
extern GE *tri;
extern Basis phi;
extern QUADRATUR QUAD;
extern int NT, NK, *nach, bandwidth;
extern REAL *XY, PI;
extern NEUTRALS ZO;

/* org

   vfak = PED.delta*PED.CV*PED.CT/PED.CL; 
   dfak = PED.delta*PED.EPS0;
   ContEqMatrix(Matrix,1.0,dfak,-vfak,vel,vfak,unu);
   PED_rhs(time,rhs,rho,v);

   // SOLUTION
   res_ke = lgs_solver(time,Matrix,rho,rhs,solver.mode,dbound,amul_bd);

*/

REAL HelmholtzEquationEmptyRoom(int loop, REAL *rho, REAL *v, REAL *one, REAL *zero[2], void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *));

//--------------------------------------------------------------------

REAL ContinuityEquation(int loop, REAL *rho, REAL *vel[2], REAL *unu, REAL *v, void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *))
{
  REAL vfak, dfak, time, res_ke;
  REAL *Matrix, *rhs;

  time = loop*PED.delta;

  Matrix = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  rhs    = (REAL *)calloc( NK , sizeof(REAL ));

#if EOC
  FuncVec(time,vel[0],U1_Exact);
  FuncVec(time,vel[1],U2_Exact);
  u_nu(unu,vel);
#endif

  vfak = PED.delta*PED.CV*PED.CT/PED.CL; 
  dfak = PED.delta*PED.EPS0;
  //  ContEqMatrix(Matrix,1.0,dfak,-vfak,vel,-dfak,vfak,unu);
  ContEqMatrix(Matrix,1.0,dfak,-vfak,vel,0.,vfak,unu);
  ContRHS(time,rhs,rho);
  
  // SOLUTION
  res_ke = lgs_solver(time,Matrix,rho,rhs,solver.mode,dbound,amul_bd);

#if WMSG  
  if(isnan(res_ke)) ErrorMessage("NaN: Possibly there is no exit.","LGS/ContinuityEquation");
#endif
  
#if EOC
  ErrorFunc(time,rho,Rho_Exact,dRho_Exact,"Continuity-R");
#endif

#if !EOC
  if(time>2)
    NullEinsBound(rho,refine.gridwidth,NK);
  else
    NullBound(rho,0.,NK);
#endif
  
  free(Matrix);
  free(rhs);

  return res_ke;
}





//Aufruf: HelmholtzEquation(ped_time,rho,phi,one,zero,PED_Helm_bound2vec);
//
REAL HelmholtzEquation(int loop, REAL *rho, REAL *v, void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *))
{
  REAL *f, *rhs, *Matrix;
  REAL res_he=-1, VBound, eps=0.01*refine.gridwidth, dummy, time;
  char fname[100];
  int i, l;

  Matrix = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  f      = (REAL *)calloc( NK , sizeof(REAL ));
  rhs    = (REAL *)calloc( NK , sizeof(REAL ));
  time = loop*PED.delta;
  
#if EOC
  FuncVec(time,rho,Rho_Exact);
#endif
  
  FD_Vec(f,rho,0);

#if EOC
  for(i=0 ; i<NK ; i++)
    f[i] = 1./f[i]/f[i];
#else
  for(i=0 ; i<NK ; i++)
    f[i] = 1./(f[i]*f[i]+EpsReg*EpsReg);
#endif
  
  HelmEqMatrix(Matrix,1.0,f,PED.EPS1);

#if EOC
  HelmRHS(time,rhs);
#endif

  // SOLUTION
  res_he = lgs_solver(time,Matrix,v,rhs,1,dbound,amul_bd);
  
#if PLOT==2
  REAL *phi, *dist, l2e;
  phi  = (REAL *)calloc( NK , sizeof(REAL ));
  dist = (REAL *)calloc( NK , sizeof(REAL ));

  SolHelm2Eikonal(v,phi);
  octaveplot(phi,"Phi",0.,1.,loop);

  ExitDistFunc_Vec(dist);
  octaveplot(dist,"Dist",0.,1.,loop);

  //  l2e = L2ErrorFunc(time,phi,ExitDistFunc);
  //  printf("Error: %f %f\n",gridwidth(),l2e);
  free(dist);
  free(phi);
#endif

#if EOC
  ErrorFunc(time,v,V_Exact,dV_Exact,"Helmholtz-V ");
#endif
  

  free(f); free(Matrix); free(rhs);

  return res_he;
}


/******************************/
/* Phi for graphics and tests */
/******************************/

REAL TaylorLog(REAL x, int N)
{
  int k;
  REAL sum=0.;

  for(k=1 ; k<=N ; k++)
    sum -= pow(1-x,k)/k;

  return sum;
}

void SolHelm2Eikonal(REAL *v, REAL *phi)
{
  REAL epsh=1.e-80, bv;
  int i;
  int count=0;
  REAL vmin=1;

  for(i=0 ; i<NK ; i++){
    if(v[i]<=0){
      count++;
      if(vmin>v[i]) vmin=v[i];
    }
    bv = sqrt(pow(v[i],2.)+pow(epsh,2.));
    phi[i] = -PED.EPS1*TaylorLog(bv,4);
  }

}

/*******************************/
/* max Phi for exit attractors */
/*******************************/


void MaxPhi()
{
  int i;
  REAL res_he, vmin;
  REAL *zero[2], *ze, *one, *v;

  v   = (REAL *)calloc( NK , sizeof(REAL ));
  ze  = (REAL *)calloc( NK , sizeof(REAL ));
  one = (REAL *)calloc( NK , sizeof(REAL ));
  
  for(i=0 ; i<2 ; i++)
    zero[i]  = (REAL *)calloc( NK , sizeof(REAL ));

  FieldCp(one,0.,one,1.,NK);

  AttractorMask(0);
  res_he = HelmholtzEquationEmptyRoom(0,ze,v,one,zero,PED_Helm_bound2vec,amul_bd_exit);

  PED.VMin = FieldMin(v,NK);


  /****************************************************/
#if PLOT==2
  REAL *phi;

  phi=(REAL *)calloc(NK , sizeof(REAL ));
  SolHelm2Eikonal(v,phi);
  
  octaveplot(v,"VEmpty",0,1.,0);
  octaveplot(phi,"PhiEmpty",0,1.,0);

  /*
  for(i=0 ; i<NK ; i++){
    phi[i] *= PED.ARMask[i];
    v[i] = pow(v[i],PED.ARMask[i]);
  }
  
  octaveplot(phi,"PhiEmptyMask",0,1.,0);
  octaveplot(v,"VEmptyMask",0,1.,0);
  */
  free(phi);
#endif
  /****************************************************/

  free(v);
  free(ze);
  free(one);
  for(i=0 ; i<2 ; i++)
    free(zero[i]);
  
}



REAL HelmholtzEquationEmptyRoom(int loop, REAL *rho, REAL *v, REAL *one, REAL *zero[2], void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *))
{
  REAL *f, *rhs, *Matrix;
  REAL res_he=-1, time;
  int i;

  Matrix = (REAL *)calloc( NK*bandwidth , sizeof(REAL ));
  f      = (REAL *)calloc( NK , sizeof(REAL ));
  rhs    = (REAL *)calloc( NK , sizeof(REAL ));
  time = loop*PED.delta;
  
  FD_Vec(f,rho,0);

  for(i=0 ; i<NK ; i++)
    f[i] = 1./(pow(f[i],2.)+pow(EpsReg,2.0));

  HelmEqMatrix(Matrix,1.0,f,PED.EPS1);
    
  // SOLUTION
  res_he = lgs_solver(time,Matrix,v,rhs,1,dbound,amul_bd);

  free(f); free(Matrix); free(rhs);

  return res_he;
}
