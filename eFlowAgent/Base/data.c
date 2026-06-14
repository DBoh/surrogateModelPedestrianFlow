#include <ped.h>

extern int NK;
extern REAL *XY, PI, EPS;

void uexact(REAL *u)
{
  int i;

  for(i=0 ; i<NK ; i++)
    u[i] = uex(i);

}


REAL uex(int i)
{
  return u_ex(XY[2*i],XY[2*i+1],100);
}


REAL u_ex(REAL x, REAL y, int flag)
{
  int l;
  REAL out=0, dummy, A[2][2], alpha;
  REAL xr, xl, yr, xm[2]={0.5,0.5};
  REAL rad;

  rad = sqrt(pow(x-xm[0],2.)+pow(y-xm[1],2.));

  alpha = PI;

  A[0][0] = cos(alpha); A[0][1] = -sin(alpha);
  A[1][0] = sin(alpha); A[1][1] = cos(alpha);

  xr = A[0][0]*(x-xm[0])+A[0][1]*(y-xm[1])+xm[0];
  yr = A[1][0]*(x-xm[0])+A[1][1]*(y-xm[1])+xm[1];

  switch (flag){
  case 0:
    out = 0;
    break;
  case 1:
    out = 1;
    break;
  case 2:
    out = x+y;
    break;
  case 3:
    out = x*x + y*y;
    break;
  case 4:
    out = x*x*x + y*y*y;
    break;
  case 5:
    out = sin(10*x)*cos(10*y);
    break;
  case 6:
    if(x>0.5)
      out = 1.;
    break;
  case 7:
    // autogrid
    out = xr;
    break;
  case 8:
    // autogrid
    out = yr;
    break;
  case 9:
    // autogrid
    out = x;
    if(rad<0.5)
      out = xr;
    break;
  case 10:
    // autogrid
    out = y;
    if(rad<0.5)
      out = yr;
    break;
  case 20:
    out = 1.;
    break;
  case 100: // PFS
    out = 0.;//0.9-0.8*x*x;
    break;
  }


  return out;
}



REAL du_ex(int l, REAL x, REAL y, int flag)
{
  REAL out=0, dummy1, dummy2;

  switch (flag){
  case 0:
    out = 0.;
    break;
  case 1:
    out = 0.;
    break;
  case 2:
    out = 1.;
    break;
  case 3:
    out = 2.*y;
    if(l==0)
      out = 2.*x;
    break;
  case 4:
    out = 3.*y*y;
    if(l==0)
      out = 3.*x*x;

    break;
  }

  return out;
}


void empty_vec(REAL *x, int N, REAL value)
{
  int i;

  for(i=0 ; i<N ; i++)
    x[i] = value;

}
