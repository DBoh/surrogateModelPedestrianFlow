/*!
  \file assign.c
  \brief basis functions
*/
#include <ped.h>

extern QUADRATUR QUAD;
extern Basis phi;
extern int NT, NK;
extern GE *tri;
extern INFOS msg;
extern REAL *XY, EpsReg, EpsZero;



void assign()
{
  REAL x1=2./3., x2=1./6., x3=x2, x4=0.;
  int i, j, m, l;


  switch (phi.DEG) {

  case 1:

    // Lineare Basisfunktionen
    phi.DOFloc = 3;
    phi.NOsamp = 3;
    // Stuetzstellen auf dem Standardsimplex

    for(i=0 ; i<6 ; i++)
      for(j=0 ; j<7 ; j++)
	phi.value[i][j]=0.;

    phi.value[0][0] = x1;
    phi.value[0][1] = x2;
    phi.value[0][2] = x2;

    phi.value[1][0] = x2;
    phi.value[1][1] = x1;
    phi.value[1][2] = x2;

    phi.value[2][0] = x2;
    phi.value[2][1] = x2;
    phi.value[2][2] = x1;

    // Massematrix auf Standardsimplex

    for(i=0 ; i<6 ; i++)
      for(j=0 ; j<6 ; j++){
	phi.mass[i][j]=0.;
	phi.stiff[i][j]=0.;
	for(l=0 ; l<6 ; l++)
	  phi.conv_mass[l][i][j]=0.;
      }

    x1 = 1./12.; x2 = 1./24.;
    phi.mass[0][0] = x1;
    phi.mass[1][1] = x1;
    phi.mass[2][2] = x1;

    phi.mass[0][1] = x2;
    phi.mass[0][2] = x2;
    phi.mass[1][0] = x2;
    phi.mass[1][2] = x2;
    phi.mass[2][0] = x2;
    phi.mass[2][1] = x2;

    phi.stiff[0][0] = 1;
    phi.stiff[0][1] = -0.5;
    phi.stiff[0][2] = -0.5;
    
    phi.stiff[1][0] = -0.5;
    phi.stiff[1][1] = 0.5;
    phi.stiff[1][2] = 0.;
    
    phi.stiff[2][0] = -0.5;
    phi.stiff[2][1] = 0.;
    phi.stiff[2][2] = 0.5;

    for(i=0 ; i<6 ; i++)
      for(j=0 ; j<2 ; j++)
	for(m=0 ; m<3 ; m++)
	  phi.gradx[i][j][m] = 0.;

    for(m=0 ; m<3 ; m++){
      phi.gradx[0][0][m] = -1.;
      phi.gradx[0][1][m] = -1.;
      
      phi.gradx[1][0][m] = 1.;
      phi.gradx[1][1][m] = 0.;
      
      phi.gradx[2][0][m] = 0.;
      phi.gradx[2][1][m] = 1.;
    }


    // phi.conv_mass
    /*
     MM1*24

ans =

     6     7     7
    -1    -2    -1
    -1    -1    -2

    phi.conv_mass[0][0][0] = 6/24;
    phi.conv_mass[0][0][1] = 7/24;
    phi.conv_mass[0][0][2] = 7/24;
    phi.conv_mass[0][1][0] = -1/24;
    phi.conv_mass[0][1][1] = -2/24;
    phi.conv_mass[0][1][2] = -1/24;
    phi.conv_mass[0][2][0] = -1/24;
    phi.conv_mass[0][2][1] = -1/24;
    phi.conv_mass[0][2][2] = -2/24;

MM2*24

ans =

    2.0000    3.0000    3.0000
    3.0000    2.0000    3.0000
   -1.0000   -1.0000   -2.0000

MM3*24

ans =

   -2.0000   -1.0000   -1.0000
    3.0000    2.0000    3.0000
    3.0000    3.0000    2.0000

    */
    
    break;

  case 2:

    // Quadratische Basisfunktionen
    phi.DOFloc = 6;
    phi.NOsamp = 3;

    x1 = 2./9.;
    x2 = -1./9.;
    x3 = 1./6.;

    // Stuetzstellen auf dem Standardsimplex
    phi.value[0][0] = x1;
    phi.value[0][1] = x2;
    phi.value[0][2] = x2;

    phi.value[1][0] = x2;
    phi.value[1][1] = x1;
    phi.value[1][2] = x2;

    phi.value[2][0] = x2;
    phi.value[2][1] = x2;
    phi.value[2][2] = x1;

    phi.value[3][0] = 2.*x1;
    phi.value[3][1] = 2.*x1;
    phi.value[3][2] = -x2;

    phi.value[4][0] = -x2;
    phi.value[4][1] = 2.*x1;
    phi.value[4][2] = 2.*x1;

    phi.value[5][0] = 2.*x1;
    phi.value[5][1] = -x2;
    phi.value[5][2] = 2.*x1;

    x1 = -5./3.;
    x2 = -1./3.;
    x3 = 2.;
    x4 = -2./3.;

    phi.gradx[0][0][0] = x1;
    phi.gradx[0][1][0] = x1;

    phi.gradx[1][0][0] = x2;
    phi.gradx[1][1][0] = 0.;

    phi.gradx[2][0][0] = 0.;
    phi.gradx[2][1][0] = x2;

    phi.gradx[3][0][0] = x3;
    phi.gradx[3][1][0] = x4;

    phi.gradx[4][0][0] = -x4;
    phi.gradx[4][1][0] = -x4;

    phi.gradx[5][0][0] = x4;
    phi.gradx[5][1][0] = x3;

    phi.gradx[0][0][1] = -x2;
    phi.gradx[0][1][1] = -x2;

    phi.gradx[1][0][1] = -x1;
    phi.gradx[1][1][1] = 0.;

    phi.gradx[2][0][1] = 0.;
    phi.gradx[2][1][1] = x2;

    phi.gradx[3][0][1] = -x3;
    phi.gradx[3][1][1] = 4.*x4;

    phi.gradx[4][0][1] = -x4;
    phi.gradx[4][1][1] = -4.*x4;

    phi.gradx[5][0][1] = x4;
    phi.gradx[5][1][1] = 0.;

    phi.gradx[0][0][2] = -x2;
    phi.gradx[0][1][2] = -x2;

    phi.gradx[1][0][2] = x2;
    phi.gradx[1][1][2] = 0.;

    phi.gradx[2][0][2] = 0.;
    phi.gradx[2][1][2] = -x1;

    phi.gradx[3][0][2] = 0.;
    phi.gradx[3][1][2] = x4;

    phi.gradx[4][0][2] = -4.*x4;
    phi.gradx[4][1][2] = -x4;

    phi.gradx[5][0][2] = 4.*x4;
    phi.gradx[5][1][2] = -x3;




    break;

  }

}



void dphi_T(REAL dphiT[6][2][3], int k)
{
  REAL mmat[2][2];
  int g1, g2, g3, i, mm, m, l;

  g1 = tri[k].VNR[0];
  g2 = tri[k].VNR[1];
  g3 = tri[k].VNR[2];

  mmat[0][0] = XY[2*g3+1] - XY[2*g1+1];
  mmat[0][1] = XY[2*g1] - XY[2*g3];
  mmat[1][0] = XY[2*g1+1] - XY[2*g2+1];
  mmat[1][1] = XY[2*g2] - XY[2*g1];

  //  det = mmat[1][1]*mmat[0][0]-mmat[0][1]*mmat[1][0];

  for(i=0 ; i<6 ; i++){
    for(m=0 ; m<3 ; m++){
      for(l=0 ; l<2 ; l++){
	dphiT[i][l][m]=0.;
	for(mm=0 ; mm<2 ; mm++)
	  dphiT[i][l][m] += phi.gradx[i][mm][m]*mmat[mm][l];
      }
    }
  }
}



void phi_loc(REAL phil[6], int k, REAL x, REAL y)
{
  REAL mmat[2][2], det, xd[2], xs[2], ax, ay, bx, by, cx, cy;
  int g1, g2, g3, i, mm, m, l;

  // Initialisierung

  for(m=0 ; m<6 ; m++)
    phil[m] = 0.;

  xs[0] = x;
  xs[1] = y;

  transform_point(xs,k,xd);


  switch (phi.DEG){

  case 1:

    phil[0] = 1. - xd[0] - xd[1];
    phil[1] = xd[0];
    phil[2] = xd[1];

    break;
  case 2:

    phil[0] = (1.-xd[0]-xd[1])*(2*(1.-xd[0]-xd[1])-1.);
    phil[1] = xd[0]*(2*xd[0]-1.);
    phil[2] = xd[1]*(2*xd[1]-1.);;
    phil[3] = 4.*xd[0]*(1.-xd[0]-xd[1]);
    phil[4] = 4.*xd[0]*xd[1];
    phil[5] = 4.*xd[1]*(1.-xd[0]-xd[1]);

    break;
  }
}


void GradPhiL(REAL DPhiL[3][2], int k)
{
  int l, i, j;
  REAL dphi[3][3], PhiA[2][2];
  REAL vert[3][2], det;

  // auf dem Standardsimplex
  dphi[0][0] = -1.;
  dphi[1][0] = 1.;
  dphi[2][0] = 0.;

  dphi[0][1] = -1.;
  dphi[1][1] = 0.;
  dphi[2][1] = 1.;

  for(l=0 ; l<2 ; l++)
    for(i=0 ; i<3 ; i++)
      vert[i][l] = XY[2*tri[k].VNR[i]+l];

  PhiA[0][0] = vert[2][1]-vert[0][1];
  PhiA[1][0] = vert[0][1]-vert[1][1];
  PhiA[0][1] = vert[0][0]-vert[2][0];
  PhiA[1][1] = vert[1][0]-vert[0][0];

  det = PhiA[0][0]*PhiA[1][1]-PhiA[1][0]*PhiA[0][1];

  for(l=0 ; l<2 ; l++)
    for(i=0 ; i<2 ; i++)
      PhiA[l][i] /= det;


  for(l=0 ; l<3 ; l++)
    for(i=0 ; i<2 ; i++){
      DPhiL[l][i]=0.;
      for(j=0 ; j<2 ; j++)
	DPhiL[l][i] += PhiA[j][i]*dphi[l][j];
    }
}

void dphi_loc(REAL dphil[6][2], int k, REAL x, REAL y)
{
  REAL mmat[2][2], det, xd[2], xs[2], ax, ay, bx, by, cx, cy;
  REAL DPhiL[3][2];
  int g1, g2, g3, i, mm, m, l;
  //     Ruecktransformation auf T

  // Initialisierung

  for(m=0 ; m<6 ; m++){
    for(l=0 ; l<2 ; l++){
      dphil[m][l] = 0.;
    }
  }

  xs[0] = x;
  xs[1] = y;

  transform_point(xs,k,xd);


  switch (phi.DEG){

  case 1:
    
    GradPhiL(DPhiL,k);
    for(i=0 ; i<3 ; i++)
      for(l=0 ; l<2 ; l++)
	dphil[i][l] = DPhiL[i][l];
    
    break;
  case 2:

    // Transformationsmatrix noch einbauen!!! s. GradPhiL

    dphil[0][0] = -3. + 4.*xd[0] + 4.*xd[1];
    dphil[0][1] = -3. + 4.*xd[0] + 4.*xd[1];

    dphil[1][0] = 4.*xd[0] - 1.;
    dphil[1][1] = 0.;

    dphil[2][0] = 0.;
    dphil[2][1] = 4.*xd[1] - 1.;

    dphil[3][0] = 4. - 8*xd[0] - 4.*xd[1];
    dphil[3][1] = -4.*xd[0];

    dphil[4][0] = 4.*xd[1];
    dphil[4][1] = 4.*xd[0];

    dphil[5][0] = -4.*xd[1];
    dphil[5][1] = 4. - 4.*xd[0] - 8*xd[1];

    break;
  }
}


/*!
  \brief yields the value of the discrete function uh at (x,y)
  where (x,y) is an element of triangle k
*/
REAL uh(int k, REAL x, REAL y, REAL *u)
{
  REAL out=0., phil[6];
  int i;

  phi_loc(phil,k,x,y);

  for(i=0 ; i<phi.DOFloc ; i++)
    out += u[tri[k].VNR[i]]*phil[i];

  return out;
}

void duh(REAL dvh[2], int k, REAL *v)
{
  int l, i;
  REAL dphi[6][2][3], det;

  dphi_T(dphi,k);
  det = detk(k);

  for(l=0 ; l<2 ; l++){
    dvh[l] = 0.;
    for(i=0 ; i<3 ; i++)
      dvh[l] += v[tri[k].VNR[i]]*dphi[i][l][0]/det;
  }

}

void duh_loc(REAL dvh[2], int k, REAL v[3])
{
  int l, i;
  REAL dphi[6][2][3], det;

  dphi_T(dphi,k);
  det = detk(k);

  for(l=0 ; l<2 ; l++){
    dvh[l] = 0.;
    for(i=0 ; i<3 ; i++)
      dvh[l] += v[i]*dphi[i][l][0]/det;
  }

}

/*
 * dvc[i] = grad v am Knoten i, CLement-Quasi-Interpolation
 */
void DuhClementVec(REAL *dvc[2], REAL *v)
{
  int k, i, l, j;
  REAL dv_loc[2], areak, *patcharea;

  patcharea = (REAL *)calloc(NK , sizeof(REAL ));
  
  for(l=0 ; l<2 ; l++)
    FieldCp(dvc[l],0.,dvc[l],0.,NK);
  
  for(k=0 ; k<NT ; k++){
    
    duh(dv_loc,k,v); // grad v_T
    areak = area(k); // |T|
    
    for(i=0 ; i<3 ; i++){
      j = tri[k].VNR[i];
      patcharea[j] += areak;  // |P|
      
      for(l=0 ; l<2 ; l++)
	dvc[l][j] += dv_loc[l]*areak;
    }
  }

  for(i=0 ; i<NK ; i++)
    for(l=0 ; l<2 ; l++)
      dvc[l][i] /= patcharea[i];

  free(patcharea);
}


void duh_vertex(REAL *u, REAL *du[2])
{
  REAL dvh[2], *v[2], *acount, karea;
  int k, i, l;

  for(l=0 ; l<2 ; l++)
    v[l] = (REAL *)calloc( NK , sizeof(REAL ));
  acount = (REAL *)calloc( NK , sizeof(REAL ));

  for(k=0 ; k<NT ; k++){
    duh(dvh,k,u);
    karea = area(k);
    for(i=0 ; i<3 ; i++){
      for(l=0 ; l<2 ; l++)
	v[l][tri[k].VNR[i]]  += dvh[l]*karea;
      acount[tri[k].VNR[i]] += karea;
    }
  }

  for(i=0 ; i<NK ; i++)
    for(l=0 ; l<2 ; l++)
      du[l][i] = v[l][i]/acount[i];


#if MSG
  for(i=0 ; i<NK ; i++)
    if(fabs(acount[i])<EpsZero)
      WriteLog("E: no counts in duh_vertex!\n","a");
#endif
    
  free(acount);
  for(l=0 ; l<2 ; l++)
    free(v[l]);

}



/*!
  \brief Transformation from Simplex \f$k\f$ into \f$\hat{k}\f$.

  \f[\hat{x}=A^{-1}\,(x-x_0)\,,{\rm where}\quad A=\left(\begin{array}{cc}
  x_1-x_0 & x_2-x_0\\
  y_1-y_0 & y_2-y_0
  \end{array}\right)\f]
*/
void transform_point(REAL x[2], int k, REAL xd[2]){TransformPoint(x,k,0,xd);}

// l auf 0
void TransformPoint(REAL x[2], int k, int l, REAL xd[2])
{
  REAL det, ax, ay, bx, by, cx, cy;
  int g1, g2, g3;

  g1 = tri[k].VNR[l];
  g2 = tri[k].VNR[(l+1)%3];
  g3 = tri[k].VNR[(l+2)%3];

  ax = XY[2*g1];
  ay = XY[2*g1+1];
  bx = XY[2*g2];
  by = XY[2*g2+1];
  cx = XY[2*g3];
  cy = XY[2*g3+1];

  det = (bx-ax)*(cy-ay)-(by-ay)*(cx-ax);
  det = 1./fabs(det);

  xd[0] = ((cy-ay)*(x[0]-ax)+(ax-cx)*(x[1]-ay))*det;
  xd[1] = ((ay-by)*(x[0]-ax)+(bx-ax)*(x[1]-ay))*det;

}

void PointTransform(REAL xd[2], int k, int l, REAL x[2])
{
  REAL det, ax, ay, bx, by, cx, cy;
  int g1, g2, g3;

  g1 = tri[k].VNR[l];
  g2 = tri[k].VNR[(l+1)%3];
  g3 = tri[k].VNR[(l+2)%3];

  ax = XY[2*g1];
  ay = XY[2*g1+1];
  bx = XY[2*g2];
  by = XY[2*g2+1];
  cx = XY[2*g3];
  cy = XY[2*g3+1];


  x[0] = (bx-ax)*xd[0] + (cx-ax)*xd[1] + ax;
  x[1] = (by-ay)*xd[0] + (cy-ay)*xd[1] + ay;

}

// T->Td
void transform_point_loc(REAL x[2], REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL xd[2])
{
  REAL det;

  det = (x2-x1)*(y3-y1)-(y2-y1)*(x3-x1);
  if(det<1.E-04)
    printf("triang degenerated\n");
  det = 1./fabs(det);

  xd[0] = ((y3-y1)*(x[0]-x1)+(x1-x3)*(x[1]-y1))*det;
  xd[1] = ((y1-y2)*(x[0]-x1)+(x2-x1)*(x[1]-y1))*det;

}

// Td->T
void transform2_point(REAL xd[2], int k, REAL x[2])
{
  REAL ax, ay, bx, by, cx, cy;
  int g1, g2, g3;

  g1 = tri[k].VNR[0];
  g2 = tri[k].VNR[1];
  g3 = tri[k].VNR[2];

  ax = XY[2*g1];
  ay = XY[2*g1+1];
  bx = XY[2*g2];
  by = XY[2*g2+1];
  cx = XY[2*g3];
  cy = XY[2*g3+1];

  x[1] = (by-ay)*xd[0] + (cy-ay)*xd[1] + ay;
  x[0] = (bx-ax)*xd[0] + (cx-ax)*xd[1] + ax;

}


void mass_elem(REAL ma[6][6], int k)
{
  int i, j, m, mm;
  REAL det=detk(k);


  for(i=0 ; i<phi.DOFloc ; i++)
    for(j=0 ; j<phi.DOFloc ; j++){
      ma[i][j]=0.;
      for(m=0 ; m<3 ; m++)
	ma[i][j] += phi.value[i][m]*phi.value[j][m]*det*QUAD.w_2d[m];
    }
}

void mass_elem_v(REAL ma[6][6], int k, REAL *v)
{
  int i, j, m, mm;
  REAL vh[3], x[2], xd[2];
  REAL det=detk(k);

  for(m=0 ; m<3 ; m++){
    mm=2-m;
    xd[0] = QUAD.st_2d[m][0];
    xd[1] = QUAD.st_2d[m][1];
    transform2_point(xd,k,x);

    vh[m] = uh(k,x[0],x[1],v);
  }


  for(i=0 ; i<phi.DOFloc ; i++)
    for(j=0 ; j<phi.DOFloc ; j++){
      ma[i][j]=0.;
      for(m=0 ; m<3 ; m++)
	ma[i][j] += phi.value[i][m]*phi.value[j][m]*det*QUAD.w_2d[m]*vh[m];
    }
}


// ko transponiert, da von rechts multipliziert wird in fe_amul
void konv_elem_v(REAL ko[6][6], int k, REAL *v[2])
{
  int m, mm, i, j, ll;
  REAL dphi[6][2][3];
  REAL vh[2][3], x[2], xd[2];

  dphi_T(dphi,k);

  for(m=0 ; m<3 ; m++){
    xd[0] = QUAD.st_2d[m][0];
    xd[1] = QUAD.st_2d[m][1];
    transform2_point(xd,k,x);

    for(ll=0 ; ll<2 ; ll++)
      vh[ll][m] = uh(k,x[0],x[1],v[ll]);
  }


  for(i=0 ; i<phi.DOFloc ; i++){
    for(j=0 ; j<phi.DOFloc ; j++){
      ko[i][j] = 0.;
      for(m=0 ; m<3 ; m++)
	ko[i][j] += (dphi[i][0][m]*vh[0][m]+dphi[i][1][m]*vh[1][m])
	  *phi.value[j][m]*QUAD.w_2d[m];
    }
  }
  
}



void stiff_elem(REAL s[6][6], int k)
{
  int l, m, mm, j;
  REAL det=detk(k);
  REAL dphi[6][2][3];

  dphi_T(dphi,k);

  for(l=0 ; l<phi.DOFloc ; l++)
    for(j=0 ; j<phi.DOFloc ; j++){
      s[j][l] = 0.;
      for(m=0 ; m<3 ; m++)
	for(mm=0 ; mm<2 ; mm++)
	  s[j][l] += dphi[l][mm][m]*dphi[j][mm][m]*QUAD.w_2d[m]/det;
    }


}


void stiff_elem_v(REAL s[6][6], int k, REAL *v)
{
  int l, m, mm, j;
  REAL vh[3], x[2], xd[2];
  REAL det=detk(k);
  REAL dphi[6][2][3];



  dphi_T(dphi,k);


  for(m=0 ; m<3 ; m++){
    xd[0] = QUAD.st_2d[m][0];
    xd[1] = QUAD.st_2d[m][1];
    transform2_point(xd,k,x);

    vh[m] = uh(k,x[0],x[1],v);
  }




  for(l=0 ; l<phi.DOFloc ; l++)
    for(j=0 ; j<phi.DOFloc ; j++){
      s[j][l] = 0.;
      for(m=0 ; m<3 ; m++)
	for(mm=0 ; mm<2 ; mm++)
	  s[j][l] += dphi[l][mm][m]*dphi[j][mm][m]*vh[m]*QUAD.w_2d[m]/det;
    }


}

/*********************************************************************

Randintegrale / Rand-Element-Matrizen

*********************************************************************/

/* 
   Randintegral ueber ganze Dreieckskanten
 */
void mass_elem_b_v(REAL ma[6][6], int k, int kj, REAL *v)
{
  int j1, j2, l, mi, mj;
  REAL stx[2], sty[2], length, vh[2];
  REAL phil[2][6], det;

  det = detk(k);
  
  for(mi=0 ; mi<6 ; mi++)
    for(mj=0 ; mj<6 ; mj++)
      ma[mi][mj] = 0.0;


  j1 = tri[k].VNR[(kj+1)%3];
  j2 = tri[k].VNR[(kj+2)%3];
  length = sqrt(pow(XY[2*j2]-XY[2*j1],2.0)+pow(XY[2*j2+1]-XY[2*j1+1],2.0));
  
  for(l=0 ; l<2 ; l++){
    stx[l] = XY[2*j1]+QUAD.st_1d[l]*(XY[2*j2]-XY[2*j1]);
    sty[l] = XY[2*j1+1]+QUAD.st_1d[l]*(XY[2*j2+1]-XY[2*j1+1]);
    vh[l]  = uh(k,stx[l],sty[l],v);
    phi_loc(phil[l],k,stx[l],sty[l]);
  }

  for(mi=0 ; mi<2 ; mi++){
    for(mj=0 ; mj<2 ; mj++){
      for(l=0 ; l<2 ; l++){
	ma[mi][mj] += vh[l]*phil[l][(kj+1+mi)%3]*phil[l][(kj+1+mj)%3]*QUAD.w_1d[l];
      }
    }
  }
}

/*
  Randintegral ueber Teile [p,q] subset dT von Randdreieckskanten
*/
void mass_elem_b_v_loc(REAL ma[6][6], int k, int kj, REAL *v, REAL p[2], REAL q[2])
{
  int j1, j2, l, mi, mj, k1, k2;
  REAL stx[2], sty[2], length, vh[2];
  REAL phil[2][6];

  for(mi=0 ; mi<6 ; mi++)
    for(mj=0 ; mj<6 ; mj++)
      ma[mi][mj] = 0.0;


  // lokale Nr. der Randknoten
  k1 = (kj+1)%3;
  k2 = (kj+2)%3;
  // globale Nr. der Randknoten
  j1 = tri[k].VNR[k1];
  j2 = tri[k].VNR[k2];
  // Laenge des Integrationsintervall
  length = sqrt(pow(p[0]-q[0],2.0)+pow(p[1]-q[1],2.0));
  
  for(l=0 ; l<2 ; l++){
    // Stuetzstellen auf dem Integrationsintervall
    stx[l] = p[0]+QUAD.st_1d[l]*(q[0]-p[0]);
    sty[l] = p[1]+QUAD.st_1d[l]*(q[1]-p[1]);
    // Werte der Integranten an den Stuetzstellen
    vh[l]  = uh(k,stx[l],sty[l],v);
    phi_loc(phil[l],k,stx[l],sty[l]);
  }

  // Quadraturformel
  for(l=0 ; l<2 ; l++){
    ma[k1][k1] += vh[l]*phil[l][k1]*phil[l][k1]*QUAD.w_1d[l]*length;
    ma[k1][k2] += vh[l]*phil[l][k1]*phil[l][k2]*QUAD.w_1d[l]*length;
    ma[k2][k1] += vh[l]*phil[l][k2]*phil[l][k1]*QUAD.w_1d[l]*length;
    ma[k2][k2] += vh[l]*phil[l][k2]*phil[l][k2]*QUAD.w_1d[l]*length;
  }

  
}

/*
  Randintegral ueber Teile [p,q] subset dT von Randdreieckskanten
  ko transponiert, da von rechts multipliziert wird
*/
void konv_elem_b_v_loc(REAL ko[6][6], int k, int kj, REAL *v, REAL p[2], REAL q[2])
{
  int j1, j2, l, mi, mj, k1, k2, i, j;
  REAL stx[2], sty[2], length, vh[2];//, Ainv[2][2];
  REAL phil[2][6], dphil[2][6][2], nu[2], len;

  //  Akinv(Ainv,k);
  
  for(mi=0 ; mi<6 ; mi++)
    for(mj=0 ; mj<6 ; mj++)
      ko[mi][mj] = 0.0;

  // lokale Nr. der Randknoten
  k1 = (kj+1)%3;
  k2 = (kj+2)%3;
  // globale Nr. der Randknoten
  j1 = tri[k].VNR[k1];
  j2 = tri[k].VNR[k2];
  // Normale
  nu[0] = (q[1]-p[1]);
  nu[1] = -(q[0]-p[0]);
  
  for(l=0 ; l<2 ; l++){
    // Stuetzstellen auf dem Integrationsintervall
    stx[l] = p[0]+QUAD.st_1d[l]*(q[0]-p[0]);
    sty[l] = p[1]+QUAD.st_1d[l]*(q[1]-p[1]);
    // Werte der Integranten an den Stuetzstellen
    vh[l]  = uh(k,stx[l],sty[l],v);
    phi_loc(phil[l],k,stx[l],sty[l]);
    dphi_loc(dphil[l],k,stx[l],sty[l]);
  }

  // Quadraturformel
  for(i=0 ; i<phi.DOFloc ; i++)
    for(j=0 ; j<phi.DOFloc ; j++){
      ko[i][j] = 0.;
      for(l=0 ; l<2 ; l++)
	ko[i][j] += vh[l]*(dphil[l][j][0]*nu[0]
			     +dphil[l][j][1]*nu[1])*phil[l][i]*QUAD.w_1d[l];
    }
}



