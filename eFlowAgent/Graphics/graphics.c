#include <ped.h>

extern VIDEO video;


void u_minmax_scale(REAL *u, REAL *us, REAL umin, REAL umax, int n)
{
  REAL eps=1.E-08;
  int i;

  //  printf("uminmax * = %f %f\n",umin,umax);

  if(fabs(umin-umax)>eps){
    for(i=0 ; i<n ; i++)
      us[i] = (u[i]-umin)/(umax-umin);
  } else {
    for(i=0 ; i<n ; i++)
      us[i] = min_REAL(max_REAL(u[i],0.),1.);
  }


}


void u_scale(REAL *u, REAL *us, int n)
{
  REAL umax=u[0], umin=umax, eps=1.E-08;
  int i;


  for(i=0 ; i<n ; i++){
    if(u[i]>umax)
      umax = u[i];
    if(u[i]<umin)
      umin = u[i];
  }

  //  printf("umin,umax = %f, %f\n",umin,umax);

  if(fabs(umin-umax)>eps){
    for(i=0 ; i<n ; i++)
      us[i] = (u[i]-umin)/(umax-umin);
  } else {
    for(i=0 ; i<n ; i++)
      us[i] = min_REAL(max_REAL(u[i],0.),1.);
  }


}





void ProfileLine(REAL ax, REAL ay, REAL bx, REAL by, REAL *r, char *fname)
{
  int i, N, kk=0, l;
  REAL out=0., h, len, xi, yi;
  REAL nu[2], rh;
  FILE *fp;

  nu[0] = by-ay; nu[1] = ax-bx;
  len = sqrt(pow(nu[0],2.)+pow(nu[1],2.));

  h  = min_REAL(refine.gridwidth*10,len/10);


  N = (int)(1+len/h);
  h = len/(N-1);

  fp = fopen(fname,"w");

  for(i=0 ; i<N ; i++){
    xi = ax+i*h*(bx-ax)/len;
    yi = ay+i*h*(by-ay)/len;

    kk = search_tri(kk,xi,yi,0);
    if(kk>=0){
      rh   = uh(kk,xi,yi,r);
      fprintf(fp,"%f %f \n",i*h/len,rh);

    } else kk=0;
  }

  fclose(fp);

}

