/*!
  \file ped_ofiles.c

  \brief routines to solve pWalk in 2d
*/

#include <ped.h>


void NullBound(REAL *w,REAL null,int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    w[i] = max_REAL(w[i],null);

}

void EinsBound(REAL *w,REAL eins,int N)
{
  int i;

  for(i=0 ; i<N ; i++)
    w[i] = min_REAL(w[i],eins);
}

void NullEinsBound(REAL *w,REAL eps,int N)
{
  int i;
  char msg[100];


  for(i=0 ; i<N ; i++){
    //    if(w[i]<-eps || w[i]>1+eps) {
    if(w[i]>1+eps) {
      sprintf(msg,"unerlaubter Wert [%.2e] - [%.2e %.2e]",w[i],-eps,1+eps);

#if MSG==2      
      Message(msg,'.',"NullEinsBound");
#endif
      //      ErrorMessage(msg,"NullEinsBound");

    }
    //    w[i] = max_REAL(min_REAL(w[i],1.),0.);
    w[i] = max_REAL(w[i],0.);
  }

}

