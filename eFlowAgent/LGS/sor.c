#include <ped.h>
/*
#include <fe_lgs.h>  // TODO - NOT FOUND
#include <fe_grid.h> // TODO - NOT FOUND
*/

extern int NK, *nach;
extern REAL *XY;
extern GN *node;

void res(REAL *, REAL *[2], REAL *[2], REAL [2]);

void sor(REAL *A, REAL *x[2], REAL *y[2], REAL omega)
{
  int i, m, j, mmax=5, l, k;
  REAL aii, aij, dummy, ci, r[2];

  for(l=0 ; l<2 ; l++){
    for(i=0 ; i<NK ; i++){
      x[l][i] = XY[2*i+l];
      if(node[i].id==-2){
  x[l][i] = u_ex(l,XY[2*i],XY[2*i+1]);
  y[l][i] = u_ex(l,XY[2*i],XY[2*i+1]);
      }
    }

    res(A,x,y,r);
    printf("SOR Residuum............................%-4.1E %-4.1E\n",r[0],r[1]);
    for(m=0 ; m<mmax ; m++){
      for(i=0 ; i<NK ; i++){

  dummy  = 0.;
  aii    = A[node[i].pointmat];
  if(node[i].id==-2)
    aii=1.;
  ci     = y[l][i]*omega/aii;

  for(k=node[i].pointmat+1 ; k<node[i+1].pointmat ; k++){
    j = nach[k];

    aij = 0.;
    if(node[j].id!=-1)
      aij  = -A[k]*omega/aii;

    dummy += aij*x[l][j];
  }

  aii = 1.-omega;
  dummy += aii*x[l][i];
  x[l][i] = ci+dummy;

  /*
  if(node[i].id==-2)
    x[l][i] = u_ex(l,XY[2*i],XY[2*i+1]);
  */
      }
      res(A,x,y,r);
      printf("SOR Residuum............................%-4.1E %-4.1E\n",r[0],r[1]);
    }
  }


}


void res(REAL *A, REAL *x[2], REAL *y[2], REAL r[2])
{
  int l, i;
  REAL *x2[2];

  for(l=0 ; l<2 ; l++)
    x2[l] = calloc(NK , sizeof(REAL));

  fe_amul2(A,x,x2);

  for(l=0 ; l<2 ; l++){
    r[l]=0.;
    for(i=0 ; i<NK ; i++){
      r[l] += pow(x[l][i]-x2[l][i],2.);
    }
    r[l] = sqrt(r[l]);
  }

}

