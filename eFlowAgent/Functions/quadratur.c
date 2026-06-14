/*!
  \file quadratur.c
*/
#include <ped.h>

extern REAL NK;
extern TRI refine;

REAL QuadSegment(int method, REAL ax, REAL ay, REAL bx, REAL by, REAL *r, REAL *v[2], int loop)
{
  int i, N, kk=0, l;
  REAL out=0., h, len, xi, yi;
  REAL nu[2], rh, *rvnu, vh[2];
  char fname[100];
  
  FILE *fp;
#if PLOT==2
  sprintf(fname,"Octave/MS_%07d.dat",loop);
  fp = fopen(fname,"w");
#endif
  
  nu[0] = by-ay; nu[1] = ax-bx;
  len = sqrt(pow(nu[0],2.)+pow(nu[1],2.));

  for(l=0 ; l<2 ; l++)
    nu[l] /= len;

  h  = min_REAL(10*refine.gridwidth,len/100);

  N = (int)(1+len/h);
  h = len/(N-1);

  rvnu = (REAL *)calloc(N , sizeof(REAL ));

  for(i=0 ; i<N ; i++){
    xi = ax+i*h*(bx-ax)/len;
    yi = ay+i*h*(by-ay)/len;

    kk = search_tri(kk,xi,yi,0);
    if(kk>=0){
      rh   = uh(kk,xi,yi,r);
      for(l=0 ; l<2 ; l++)
	vh[l] = uh(kk,xi,yi,v[l]);

      rvnu[i] = rh*(vh[0]*nu[0]+vh[1]*nu[1]);

#if PLOT==2
      fprintf(fp,"%f %f %f %f\n",i*h/len,rh,sqrt(vh[0]*vh[0]+vh[1]*vh[1]),rvnu[i]);
#endif

    } else kk=0;
  }

#if PLOT==2
  fclose(fp);
#endif

  switch (method) {
  case 0:

    for(i=0 ; i<N-1 ; i++)
      out += h*rvnu[i];

    break;
  case 1:

    for(i=0 ; i<N-1 ; i++)
      out += 0.5*h*(rvnu[i]+rvnu[i+1]);

    break;
  }


  free(rvnu);

  return out;
}

