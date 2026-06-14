#include <ped.h>

extern DAT PED;
extern REAL *XY, EpsZero, EpsReg;
extern int NK, bandwidth;
extern REAL PI;
extern GN *node;
extern MATRIX MAT;
extern NEUTRALS ZO;

// g^C_R im pFLow.pdf
void G_Cont_Exact_Vec(REAL time, REAL *g)
{
  REAL xx, yy, fac;
  int i;

  fac = PED.CV*PED.CT/PED.CL;
  for(i=0 ; i<NK ; i++){
    xx = XY[2*i]; yy=XY[2*i+1];
    // rho_t
    g[i]  = dRho_Exact(time,xx,yy,-1);
    // -eps_0*Laplace rho
    g[i] -= PED.EPS0*d2Rho_Exact(time,xx,yy,0);
    g[i] -= PED.EPS0*d2Rho_Exact(time,xx,yy,3);
    // + div(rho*u) = rho_x*u1+rho*u1_x+rho_y*u2+rho*u2_y
    g[i] += dRho_Exact(time,xx,yy,0)*U1_Exact(time,xx,yy)*fac;
    g[i] += Rho_Exact(time,xx,yy)*dU1_Exact(time,xx,yy,0)*fac;
    g[i] += dRho_Exact(time,xx,yy,1)*U2_Exact(time,xx,yy)*fac;
    g[i] += Rho_Exact(time,xx,yy)*dU2_Exact(time,xx,yy,1)*fac;
  }
  
}


// g^H_R im pFLow.pdf
void G_Helm_Exact_Vec(REAL time, REAL *g)
{
  int i;
  REAL *f, *rho;

  f   = (REAL *)calloc(NK , sizeof(REAL ));
  rho = (REAL *)calloc(NK , sizeof(REAL ));
  
  FuncVec(time,rho,Rho_Exact);
  
  FD_Vec(f,rho,0);

  for(i=0 ; i<NK ; i++){
    if(fabs(f[i])<EpsZero) ErrorMessage("division by zero","G_Exact_Vec");
    g[i]  = V_Exact(time,XY[2*i],XY[2*i+1])/f[i]/f[i];
    g[i] -= PED.EPS1*PED.EPS1*d2V_Exact(time,XY[2*i],XY[2*i+1],0);
    g[i] -= PED.EPS1*PED.EPS1*d2V_Exact(time,XY[2*i],XY[2*i+1],3); 
  }

  free(f);
  free(rho);
  
}


void G_Helm_Exact_Entree_Vec(REAL time, REAL *g)
{
  int i;
  
  FieldCp(g,0.,g,0.,NK);
  
  for(i=0 ; i<NK ; i++){
    if(node[i].id<0 && ChkEntree(i)){
      g[i]  = dV_Exact(time,XY[2*i],XY[2*i+1],0)*node[i].WallNu[0];
      g[i] += dV_Exact(time,XY[2*i],XY[2*i+1],1)*node[i].WallNu[1];
      g[i] += V_Exact(time,XY[2*i],XY[2*i+1])/PED.EPS1;
    }
  }

}

// rechte Seite fuer die Helmhotzgleichung
void HelmRHS(REAL time, REAL *rhs)
{
  REAL *ghr, *rhs2;
  ghr  = (REAL *)calloc(NK , sizeof(REAL ));
  rhs2 = (REAL *)calloc(NK , sizeof(REAL ));
  
  FieldCp(rhs,0.,rhs,0.,NK);
  
  G_Helm_Exact_Vec(time,ghr);
  amul(MAT.Mass,ghr,rhs);
  
  if(PED.numentree){
    REAL *ghe, *ME;
    ghe  = (REAL *)calloc(NK , sizeof(REAL ));
    ME   = (REAL *)calloc(NK*bandwidth , sizeof(REAL ));
    
    G_Helm_Exact_Entree_Vec(time,ghe); 
    BoundIntegral(ME,ZO.NKones,PED.numentree,PED.entree);
    amul(ME,ghe,rhs2);

    field_cp(rhs,1.0,rhs,PED.EPS1*PED.EPS1,rhs2,0.,NK);
    
    free(ghe); free(ME);
  }


  free(ghr);
  free(rhs2);
}


void Exact_Vec(REAL t, REAL *g, REAL (*func)(REAL, REAL , REAL))
{
  int i;

  for(i=0 ; i<NK ; i++)
    g[i] = func(t,XY[2*i],XY[2*i+1]);
}

REAL Rho_Exact(REAL t, REAL x, REAL y)
{
  return 26*(cos(y/PI+PI)+1)*(x+0.5)*(x+0.5);
}

REAL dRho_Exact(REAL t, REAL x, REAL y, int l)
{
  REAL out=0;

  switch (l){
  case 0:
    return 2*26*(cos(y/PI+PI)+1)*(x+0.5);
    break;
  case 1:
    return -26/PI*sin(y/PI+PI)*(x+0.5)*(x+0.5);
    break;
  default: // Zeitableitung
    return 0;
    break;
  }
  
  ErrorMessage("unknown l","dRho_Exact");
  return 0;
}

REAL d2Rho_Exact(REAL t, REAL x, REAL y, int ij)
{
  REAL out=0;

  switch (ij){
  case 0: // xx
    return 2*26*(cos(y/PI+PI)+1);
    break;
  case 1: case 2: // xy
    return -2*26/PI*sin(y/PI+PI)*(x+0.5);
    break;
  default: // yy
    return -26/PI/PI*cos(y/PI+PI)*(x+0.5)*(x+0.5);
    break;
  }

  ErrorMessage("unknown ij","d2Rho_Exact");
  return 0;
}

// Pin = (eps_0*nabla rho - rho*u)*nu (Integral: -0.281575653835830)
void P_In_Exact_Vec(REAL t, REAL *PIn)
{
  int i, l;
  REAL rho_exact, drho_exact[2], u_exact[2], fak;

  FieldCp(PIn,0.0,PIn,0.0,NK);
  
  fak = PED.CV*PED.CT/PED.CL;
  
  for(i=0 ; i<NK ; i++){
    if(node[i].id<0 && ChkEntree(i)){
      for(l=0 ; l<2 ; l++)
	drho_exact[l] = dRho_Exact(t,XY[2*i],XY[2*i+1],l);

      rho_exact = Rho_Exact(t,XY[2*i],XY[2*i+1]);
      u_exact[0] = U1_Exact(t,XY[2*i],XY[2*i+1]);
      u_exact[1] = U2_Exact(t,XY[2*i],XY[2*i+1]);

      PIn[i] = (-PED.EPS0*drho_exact[0]+fak*rho_exact*u_exact[0])*node[i].WallNu[0];
      PIn[i] += (-PED.EPS0*drho_exact[1]+fak*rho_exact*u_exact[1])*node[i].WallNu[1];
      PIn[i] = -PIn[i];
    }
  }

#if PLOT==2  
  octaveplot(PIn,"PIN",0.,-1.,0);
#endif
  
}


REAL U1_Exact(REAL t, REAL x, REAL y)
{
  int l;
  REAL out=0, dv[2], bdv;

  for(l=0 ; l<2 ; l++)
    dv[l] = dV_Exact(t,x,y,l);
  bdv = dv[0]*dv[0]+dv[1]*dv[1];
  bdv = 1./sqrt(bdv);
  
  out  = FD_Value(Rho_Exact(t,x,y),0,x,y,PED.FD); // f(rho)
  
  out *= dv[0];
  out *= bdv;
  
  return out;
}

REAL U2_Exact(REAL t, REAL x, REAL y)
{
  int l;
  REAL out=0, dv[2], bdv;

  for(l=0 ; l<2 ; l++)
    dv[l] = dV_Exact(t,x,y,l);
  bdv = dv[0]*dv[0]+dv[1]*dv[1];
  bdv = 1./sqrt(bdv);
  
  out  = FD_Value(Rho_Exact(t,x,y),0,x,y,PED.FD);
  
  out *= dv[1];
  out *= bdv;
  
  return out;
}

REAL dU1_Exact(REAL t, REAL x, REAL y, int flag)
{
  int l, i;
  REAL out=0., f, df, drho, dv[2], Hv[2][2], bdv, out1=0., out2=0.;

  drho = dRho_Exact(t,x,y,flag); // rho_xi
  f    = FD_Value(Rho_Exact(t,x,y),0,x,y,1); //f(rho)
  df   = FD_Value(Rho_Exact(t,x,y),1,x,y,1); //f'(rho)
  for(l=0 ; l<2 ; l++){
    dv[l] = dV_Exact(t,x,y,l);
    for(i=0 ; i<2 ; i++)
      Hv[l][i] = d2V_Exact(t,x,y,2*l+i);
  }
  bdv = sqrt(dv[0]*dv[0]+dv[1]*dv[1]);

  out1 = df*drho*dv[0]/bdv;

  out2  = dv[0]*Hv[0][flag]+dv[1]*Hv[1][flag]; // dv*Hv_0
  out2 *= dv[0]/(pow(bdv,3.));
  
  out2  = Hv[0][flag]/bdv - out2;
  out2 *= f;

  out  = out1+out2;


  return out;
}

REAL dU2_Exact(REAL t, REAL x, REAL y, int flag)
{
  int l, i;
  REAL out=0., f, df, drho, dv[2], Hv[2][2], bdv, out1=0., out2=0.;

  drho = dRho_Exact(t,x,y,flag); // rho_xi
  f    = FD_Value(Rho_Exact(t,x,y),0,x,y,1); //f(rho)
  df   = FD_Value(Rho_Exact(t,x,y),1,x,y,1); //f'(rho)
  for(l=0 ; l<2 ; l++){
    dv[l] = dV_Exact(t,x,y,l);
    for(i=0 ; i<2 ; i++)
      Hv[l][i] = d2V_Exact(t,x,y,2*l+i);
  }
  bdv = sqrt(dv[0]*dv[0]+dv[1]*dv[1]);

  out1 = df*drho*dv[1]/bdv;

  out2  = dv[0]*Hv[0][flag]+dv[1]*Hv[1][flag]; // dv*Hv_0
  out2 *= dv[1]/(pow(bdv,3.));
  
  out2  = Hv[1][flag]/bdv - out2;
  out2 *= f;

  out  = out1+out2;

  return out;
}

REAL V_Exact(REAL t, REAL x, REAL y)
{
  
  return 10./36.*(x+0.1)*(x+0.1)+1./2.-y*y;
  
}

REAL dV_Exact_Vec(REAL t, REAL *g[2], REAL (*func)(REAL, REAL , REAL, int))
{
  int i, l;

  for(i=0 ; i<NK ; i++)
    for(l=0 ; l<2 ; l++)
      g[l][i] = func(t,XY[2*i],XY[2*i+1],l);
}

REAL dV_Exact(REAL t, REAL x, REAL y, int flag)
{
  
  if(flag==0) // _x
    return 10./18.*(x+0.1);
  else        // _y
    return -2*y;

}

REAL d2V_Exact(REAL t, REAL x, REAL y, int ij)
{
  REAL out=0;

  switch (ij){
  case 0: // xx
    out = 10./18.;
    break;
  case 1: case 2: // xy
    out = 0.;
    break;
  default: // yy
    out = -2;
    break;
  }
  
  return out;
}


