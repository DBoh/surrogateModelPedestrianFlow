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
extern REAL PI, EpsReg;

REAL v_exact(REAL t, REAL x, REAL y)
{
  return x*x+y*y;
}

REAL dv_exact(REAL t, REAL x, REAL y, int flag)
{
  if(flag==0)
    return 2*x;
  else
    return 2*y;
}


void ClementInterpolationTest()
{
  int k, l, i;
  REAL *v, *dv[2], *dve[2], *dvn[2], *dvne[2];
  REAL L2e, H1e, Error1, Error2, nn;

  v  = (REAL*)calloc(NK , sizeof(REAL ));
  for(l=0 ; l<2 ; l++){
    dv[l]    = (REAL*)calloc(NK , sizeof(REAL ));
    dve[l]   = (REAL*)calloc(NK , sizeof(REAL ));
    dvn[l]   = (REAL*)calloc(NK , sizeof(REAL ));
    dvne[l]  = (REAL*)calloc(NK , sizeof(REAL ));
  }

  for(i=0 ; i<NK ; i++){
    v[i] = v_exact(0,XY[2*i],XY[2*i+1]);

    for(l=0 ; l<2 ; l++)
      dve[l][i] = dv_exact(0,XY[2*i],XY[2*i+1],l);
    
    nn = sqrt(pow(dve[0][i],2.)+pow(dve[1][i],2.)+pow(EpsReg,2.));

    for(l=0 ; l<2 ; l++)
      dvne[l][i] = dve[l][i]/nn;
    
  }
  // interpolierte Funktion
  
  // interpolierter Gradient
  duh_vertex(v,dv);

  for(i=0 ; i<NK ; i++){
    
    nn = sqrt(pow(dv[0][i],2.)+pow(dv[1][i],2.)+pow(EpsReg,2.));

    for(l=0 ; l<2 ; l++)
      dvn[l][i] = dv[l][i]/nn;
    
  }  

  Error1 = 0;
  Error2 = 0;
  for(i=0 ; i<NK ; i++){
    Error1 += pow(dv[0][i]-dve[0][i],2.)+pow(dv[1][i]-dve[1][i],2.);
    Error2 += pow(dvn[0][i]-dvne[0][i],2.)+pow(dvn[1][i]-dvne[1][i],2.);
  }
  
  Error1 = sqrt(Error1/NK);
  Error2 = sqrt(Error2/NK);

  printf("%e %e %e\n",refine.gridwidth,Error1,Error2);

  ErrorFunc(0,v,v_exact,dv_exact,"FEM-Interpolation");
  
  //  L2e = L2ErrorFunc(0,v,v_exact);

  free(v);
  for(l=0 ; l<2 ; l++){
    free(dv[l]);
    free(dve[l]);
  }
}
