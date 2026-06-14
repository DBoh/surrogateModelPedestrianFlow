/*!
  \file ped_fundamental.c

  \brief several fundamental diagrams
*/
#include <ped.h>


REAL fd_weidmann_lin(REAL *FD[2], int N, REAL rho)
{
  int i;
  REAL out=0., y2, y1, x2, x1;

  if(rho>=0.0 && rho<=FD[0][0]) return FD[1][0];

  for(i=0 ; i<N ; i++)
    if(rho>=FD[0][i] && rho<=FD[0][i+1] ){
      x1 = FD[0][i];
      x2 = FD[0][i+1];
      y1 = FD[1][i];
      y2 = FD[1][i+1];
      out = (y2*(rho-x1)+y1*(x2-rho))/(x2-x1);
      return out;
    }

  return out;
}




