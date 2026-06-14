/*!
  \file ped_boundary.c
  \brief boundary connectivities
*/
#include <ped.h>

extern DAT PED;
extern REAL GridEpsZero;
extern GN *node;
extern REAL *XY;
extern int NT, NK;
extern GE *tri;
extern TRI refine;


//! |Gamma_A(flag>=0)| OR sum|Gamma_A(flag<0)|
REAL DoorLength(int NumDoors, REAL *Door[4], int flag)
{
  int l;
  REAL X1, X2, Y1, Y2, out=0.;

  if(flag<0){

    for(l=0 ; l<NumDoors ; l++){
      X1 = Door[0][l]; Y1=Door[1][l];
      X2 = Door[2][l]; Y2=Door[3][l];
      out += sqrt(pow(X1-X2,2.)+pow(Y1-Y2,2.));
    }

  } else {
    X1 = Door[0][flag]; Y1=Door[1][flag];
    X2 = Door[2][flag]; Y2=Door[3][flag];
    out = sqrt(pow(X1-X2,2.)+pow(Y1-Y2,2.));
  }

  return out;
}

// Des
void Entree(REAL *u, REAL time)
{
  int i, l=0;
  REAL xx, tt, t1, t2;

  if(time>=3.){
    field_cp(u,0.,u,0.,u,0.,NK);
  } else {

  for(i=0 ; i<NK ; i++)
    if(PED_entree(node[i].id)){

      l = (int)(-(node[i].id+4)/2);

      t2 = sqrt(pow(PED.entree[0][l]-PED.entree[2][l],2.)+
    pow(PED.entree[1][l]-PED.entree[3][l],2.));
      t1 = sqrt(pow(PED.entree[0][l]-XY[2*i],2.)+
    pow(PED.entree[1][l]-XY[2*i+1],2.));

      tt=t1/t2;

      //      u[i] = PED.pFlowDoor*4.*tt*(1.-tt);
      u[i] = 4.*tt*(1.-tt);

    }
  }
}

int QuaderBound(REAL X1,REAL Y1,REAL X2,REAL Y2,REAL X,REAL Y)
{
  int out=0, l;
  REAL tau[2], nu[2], delta=0.5*refine.gridwidth, nt, tpn[2], tmn[2];
  REAL q1[2], q2[2], q3[2], q4[2];
  REAL a1, a2, a3, a4, aq;

  tau[0] = X2-X1; tau[1] = Y2-Y1;
  nt = norm(tau,2,2.);

  for(l=0 ; l<2 ; l++)
    tau[l] /= nt;

  nu[0] = tau[1]; nu[1] = -tau[0];

  for(l=0 ; l<2 ; l++){
    tpn[l] = tau[l]+nu[l];
    tmn[l] = tau[l]-nu[l];
  }

  // Quader um Ausgangsdaten festlegen
  q1[0] = X1 - delta*tpn[0]; q1[1] = Y1 - delta*tpn[1];
  q2[0] = X1 - delta*tmn[0]; q2[1] = Y1 - delta*tmn[1];
  q3[0] = X2 + delta*tpn[0]; q3[1] = Y2 + delta*tpn[1];
  q4[0] = X2 + delta*tmn[0]; q4[1] = Y2 + delta*tmn[1];

  // Pruefen, ob (X,Y) in diesem Quader enthalten ist
  aq = fabs(area_loc(q1[0],q1[1],q2[0],q2[1],q3[0],q3[1]))+
    fabs(area_loc(q4[0],q4[1],q1[0],q1[1],q3[0],q3[1]));     // |Q|

  a1 = fabs(area_loc(X,Y,q1[0],q1[1],q2[0],q2[1]));
  a2 = fabs(area_loc(X,Y,q3[0],q3[1],q2[0],q2[1]));
  a3 = fabs(area_loc(X,Y,q3[0],q3[1],q4[0],q4[1]));
  a4 = fabs(area_loc(X,Y,q1[0],q1[1],q4[0],q4[1]));

  printf("%f %f %e\n",aq,a1+a2+a3+a4,fabs(aq-(a1+a2+a3+a4)));

  if(fabs(aq-(a1+a2+a3+a4))<2e-04) //GridEpsZero)
    out = 1;

  return out;
}



// Des
int ChkWall(int i)
{
  if(!ChkEntree(i) && !ChkExit(i) && node[i].id<0)
    return 1;
  return 0;
}

// Des
int ChkEntree(int i)
{
  if(abs(node[i].id)%2==0 && node[i].id<=-4)
    return (int)(-(node[i].id+4))/2+1;
  return 0;
}

// Des
int ChkExit(int i)
{
  if(abs(node[i].id)%2!=0 && node[i].id<=-3)
    return (int)(-(node[i].id+3)/2)+1;
  return 0;
}


int ChkExitPoint(REAL x, REAL y)
{
  int l;
  REAL P[2], E[2], F[2];

  for(l=0 ; l<PED.numexit ; l++){
    P[0] = x-PED.exit[0][l];
    P[1] = y-PED.exit[1][l];
    E[0] = x-PED.exit[2][l];
    E[1] = y-PED.exit[3][l];
    F[0] = PED.exit[0][l]-PED.exit[2][l];
    F[1] = PED.exit[1][l]-PED.exit[3][l];
    if(fabs(norm(P,2,2.)+norm(E,2,2.)-norm(F,2,2.))<GridEpsZero) return 1;
  }

  return 0;
}

int ChkEntreePoint(REAL x, REAL y)
{
  int l;
  REAL P[2], E[2], F[2];

  for(l=0 ; l<PED.numentree ; l++){
    P[0] = x-PED.entree[0][l];
    P[1] = y-PED.entree[1][l];
    E[0] = x-PED.entree[2][l];
    E[1] = y-PED.entree[3][l];
    F[0] = PED.entree[0][l]-PED.entree[2][l];
    F[1] = PED.entree[1][l]-PED.entree[3][l];
    if(fabs(norm(P,2,2.)+norm(E,2,2.)-norm(F,2,2.))<GridEpsZero) return 1;
  }

  return 0;
}


void PED_Eikonal_bound2vec(REAL time, REAL *v, int flag)
{
  int i, ne;
  REAL dummy=-1.0/PED.EPS1;

  switch (flag) {
  case 1:
    // hier der Dirichletrand
    for(i=0 ; i<NK ; i++){
      ne=ChkExit(i);
      if(node[i].id<0 && ne)
      	v[i] = 0.; //PED.ExitAttraction[ne-1]*(1-PED.VMin)+PED.VMin;
    }
    
    break;
  default:

    for(i=0 ; i<NK ; i++)
      if(node[i].id<0 && ChkExit(i))
	v[i] = 0.;

  }

}


// Des
void PED_Helm_bound2vec(REAL time, REAL *v, int flag)
{
  int i, ne;
  REAL dummy=-1.0/PED.EPS1;

  switch (flag) {
  case 1:
    // hier der Dirichletrand
    for(i=0 ; i<NK ; i++){
      if(node[i].id<0){
	ne=ChkExit(i);
	if(ne){
#if EOC
	  v[i] = V_Exact(time,XY[2*i],XY[2*i+1]);
#else  
	  v[i] = PED.ExitAttraction[ne-1]*(1-PED.VMin)+PED.VMin;
#endif
	}
      }
    }
    break;
  default:

    for(i=0 ; i<NK ; i++)
      if(node[i].id<0 && ChkExit(i))
	v[i] = 0.;

  }
}


// dirichlet boundary for eoc
void PED_Kont_bound2vec(REAL time, REAL *v, int flag)
{
  int i, na;
  REAL dummy=-1.0/PED.EPS1, Vnu;

  switch (flag) {
  case 1:
    // hier der Dirichletrand
    for(i=0 ; i<NK ; i++){
      if(node[i].id<0){
	na=ChkEntree(i);
	if(na)
	  if(PED.MaxEntreePersons[na-1]>PED.EntreePersons[na-1]*time)
	    v[i] = PED.DensityDoor[na-1];
      }
    }
    break;
  default:

    for(i=0 ; i<NK ; i++)
      if(node[i].id<0 && ChkEntree(i))
	v[i] = 0.;

  }



#if EOC
  int i;
  for(i=0 ; i<NK ; i++)
    if(fabs(y-0.5)<EpsZero)
      v[i] = Rho_Exact(time,XY[2*i],XY[2*i+1]);
#endif
  
}



void PED_Helm_EmptyRoom_bound2vec(REAL time, REAL *v, int flag)
{
  int i;

  for(i=0 ; i<NK ; i++)
    if(node[i].id<0 && ChkExit(i))
      v[i] = 1.;
  

}



