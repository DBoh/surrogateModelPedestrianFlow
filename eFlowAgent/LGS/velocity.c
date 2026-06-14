/*!
  \file velocity.c
  \brief all about velocity computations
*/
#include <ped.h>
extern DAT PED;
extern int NK;
extern GN *node;
extern REAL EpsReg, *XY, EpsZero;
extern MATRIX MAT;
extern NEUTRALS ZO;

void slip_boundary(REAL *[2]);
void VelocityVec(REAL *[2], REAL *, REAL *);
void VelocityVecPhi(REAL *vel[2], REAL *rho, REAL *phi);
void u_nu(REAL *, REAL *[2]);
void EntreeVel(REAL *[2]);
void EntreeFlow(REAL *[2]);


void UpdateVelocity(int LOOP, REAL *vel[2], REAL *rho, REAL *v, REAL *unu)
{
  int i;
  
  AttractorMask(LOOP);

#if EOC
  Exact_Vec(0.,rho,Rho_Exact);
  Exact_Vec(0.,v,V_Exact);
#else
  for(i=0 ; i<NK ; i++)
    v[i] = pow(v[i],PED.ARMask[i]);
#endif
  
  VelocityVec(vel,rho,v);

#if EOC
  ErrorVecFunc(LOOP*PED.delta,vel,U1_Exact,U2_Exact,dU1_Exact,dU2_Exact,"Velocity    ");
#endif

  slip_boundary(vel);
  //  EntreeVel(vel);
  u_nu(unu,vel);

  if(PED.entree)
    EntreeFlow(vel);
  
#if PLOT==2
  octaveplot(unu,"UNU",0.,-1,LOOP);
#endif
}

  

void UpdateVelocityPhi(int LOOP, REAL *vel[2], REAL *rho, REAL *phi, REAL *unu)
{
  int i, l;
  
  AttractorMask(LOOP);

#if EOC
  Exact_Vec(0.,rho,Rho_Exact);
  Exact_Vec(0.,v,V_Exact);
#else
  /*
  for(i=0 ; i<NK ; i++) 
    phi[i] = pow(v[i],PED.ARMask[i]);
  */
#endif
  
  VelocityVec(vel,rho,phi);
  for(l=0 ; l<2 ; l++)
    FieldCp(vel[l],-1.,vel[l],0.,NK);

#if EOC
  ErrorVecFunc(LOOP*PED.delta,vel,U1_Exact,U2_Exact,dU1_Exact,dU2_Exact,"Velocity    ");
#endif

  slip_boundary(vel);
  //  EntreeVel(vel);
  u_nu(unu,vel);

  if(PED.entree)
    EntreeFlow(vel);
  
#if PLOT==2
  octaveplot(unu,"UNU",0.,-1,LOOP);
#endif
}

  
void u_nu(REAL *unu, REAL *u[2])
{
  int j, k, l, j1, j2;
  REAL bn, nuk[2];

  FieldCp(unu,0.,unu,0.,NK);

  for(j=0 ; j<NK ; j++){
    if(node[j].id<0){
      unu[j] = u[0][j]*node[j].WallNu[0]+u[1][j]*node[j].WallNu[1];
    }
  }

}





void slip_boundary(REAL *v[2])
{

  int l, j, k, j1, j2, je;
  REAL tau[2], vt;

  for(j=0 ; j<NK ; j++){
    if(ChkWall(j)){

      tau[0] = -node[j].WallNu[1]; 
      tau[1] = node[j].WallNu[0];
      vt = v[0][j]*tau[0]+v[1][j]*tau[1];

      for(l=0 ; l<2 ; l++)
	v[l][j] = vt*tau[l];

    }
  }

}


/* ----------------------------------------------------------*/

void VelocityVec(REAL *vel[2], REAL *rho, REAL *v)
{
  int i, l;
  REAL *f, *dv[2], bdv;

  f   = (REAL *)calloc(NK , sizeof(REAL ));
  for(l=0 ; l<2 ; l++)
    dv[l] = (REAL *)calloc(NK , sizeof(REAL ));
  

  FD_Vec(f,rho,0);

#if EOC
  dV_Exact_Vec(0.,dv,dV_Exact);
#else
  DuhClementVec(dv,v);
#endif
  
  for(i=0 ; i<NK ; i++){
#if EOC
    bdv = sqrt(dv[0][i]*dv[0][i]+dv[1][i]*dv[1][i]);
#else
    bdv = sqrt(dv[0][i]*dv[0][i]+dv[1][i]*dv[1][i]+EpsReg*EpsReg);
    bdv = sqrt(dv[0][i]*dv[0][i]+dv[1][i]*dv[1][i]);
#endif

if (bdv < PED.empty*1e-02)
    bdv = 0;
else
    bdv = 1./bdv;

    
    for(l=0 ; l<2 ; l++)
      vel[l][i] = f[i]*dv[l][i]*bdv;
	
  }
  
  free(f);
  for(l=0 ; l<2 ; l++)
    free(dv[l]);
}

void VelocityVecPhi(REAL *vel[2], REAL *rho, REAL *phi)
{
  int i, l;
  REAL *f, *dphi[2], bdphi;

  f   = (REAL *)calloc(NK , sizeof(REAL ));
  for(l=0 ; l<2 ; l++)
    dphi[l] = (REAL *)calloc(NK , sizeof(REAL ));
  

  FD_Vec(f,rho,0);

#if EOC
  //  dV_Exact_Vec(0.,dv,dV_Exact);
#else
  DuhClementVec(dphi,phi);
#endif
  
  for(i=0 ; i<NK ; i++){
#if EOC
    //    bdv = sqrt(dv[0][i]*dv[0][i]+dv[1][i]*dv[1][i]);
#else
    bdphi = sqrt(dphi[0][i]*dphi[0][i]+dphi[1][i]*dphi[1][i]+EpsReg*EpsReg);
    bdphi = sqrt(dphi[0][i]*dphi[0][i]+dphi[1][i]*dphi[1][i]);
#endif

if (bdphi < PED.empty*1e-02)
    bdphi = 0;
else
    bdphi = 1./bdphi;

    
    for(l=0 ; l<2 ; l++)
      vel[l][i] = -f[i]*dphi[l][i]*bdphi;
	
  }
  
  free(f);
  for(l=0 ; l<2 ; l++)
    free(dphi[l]);
}


/* at entrees */

void u_entree_nu(REAL *unu, REAL *u[2])
{
  int j, k, l, j1, j2;
  REAL bn, nuk[2];

  FieldCp(unu,0.,unu,0.,NK);

  for(j=0 ; j<NK ; j++){
    if(node[j].id<0 && ChkEntree(j)){
      unu[j] = u[0][j]*node[j].EntreeNu[0]+u[1][j]*node[j].EntreeNu[1];
    }
  }

}


void EntreeFlow(REAL *vel[2])
{
  int l, i;
  REAL *unu, *EntreeOnes, VE, vfaki, RhoIn;

  EntreeOnes = (REAL *)calloc(NK , sizeof(REAL ));
  unu = (REAL *)calloc(NK , sizeof(REAL ));
  u_entree_nu(unu,vel);

  vfaki = PED.CT/PED.CL/PED.CP/PED.CV;

  for(l=0 ; l<PED.numentree ; l++){

    FieldCp(EntreeOnes,0.,EntreeOnes,0.,NK);

    for(i=0 ; i<NK ; i++)
      if(node[i].id<0 && ChkEntree(i)==l+1)
	EntreeOnes[i] = 1;
    
    VE=VecVecIntegral(MAT.MassEntree,unu,EntreeOnes);
    RhoIn = -vfaki*PED.EntreePersons[l]/VE;
    RhoIn = max_REAL(0,min_REAL(1,RhoIn));
    if(RhoIn>1.+EpsZero || RhoIn<0.-EpsZero)
      ErrorMessage("Pin out of allowed area [0,1]","EntreeFlow");
    PED.DensityDoor[l] = min_REAL(max_REAL(0.,RhoIn),1.-EpsZero);

  }

  free(EntreeOnes);
  free(unu);
  
}

void EntreeVel(REAL *vel[2])
{
  int i, l;

  for(i=0 ; i<NK ; i++)
    if(ChkEntree(i))
      for(l=0 ; l<2 ; l++)
	vel[l][i] = -node[i].WallNu[l];

}
