/*!
  \file swarmmove.c
  \brief swarmtype movements
*/
#include <ped.h>
#include <time.h>

extern VIDEO video;


void swarmmove()
{
  /*
  REAL xi, yi, R1, R2, v0[2], v1[2], v2[2], RR, n1=0., n2=0., n0=0.;
  REAL ww[3], vv[2], nvv=0.;
  int c1=0, c2=0;
  R1 = 1./PED.CL;
  R2 = 3./PED.CL;
  v0[0]=0; v0[1]=0; v1[0]=0; v1[1]=0; v2[0]=0; v2[1]=0;
  ww[1] = 2./3; ww[2] = 1./3; ww[0] = 1-ww[1]-ww[2];
    
  for(i=0 ; i<video.TrackPerson ; i++){
    if(video.TrackInside[i]){
      xi = video.TrackXYPosition[0][i];
      yi = video.TrackXYPosition[1][i];
      
      RR = (xx-xi)*(xx-xi)+(yy-yi)*(yy-yi);
      if(RR<=R1){
	  
	for(l=0 ; l<2 ; l++)
	  v1[l] += video.TrackXYPosition[l][i];
	  
	c1++;
      } else if (RR<=R2){
	  
	for(l=0 ; l<2 ; l++)
	  v2[l] += video.TrackXYPosition[l][i];
	
	c2++;
      }
    }
  }
  // mean positions
  for(l=0 ; l<2 ; l++){
    v0[l] = d_uh[l];
    v1[l] /= c1;
    v2[l] /= c2;
  }
  
  v1[0] = xx-v1[0];
  v1[1] = yy-v1[1];
  v2[0] = v2[0]-xx;
  v2[1] = v2[1]-yy;
    
  for(l=0 ; l<2 ; l++){
    n0 += v0[l]*v0[l];
    n1 += v1[l]*v1[l];
    n2 += v2[l]*v2[l];
  }

  n0 = sqrt(n0); n1 = sqrt(n1); n2 = sqrt(n2);

  for(l=0 ; l<2 ; l++){
    v0[l] /= n0;
    v1[l] /= n1;
    v2[l] /= n2;
      
    vv[l] = ww[0]*v0[l]+ww[1]*v1[l]+ww[2]*v2[l];
    
    nvv += vv[l]*vv[l];
  }

  for(l=0 ; l<2 ; l++)
    vv[l] *= n0/nvv;

  vstepx = vv[0];
  vstepy = vv[1];
  */
}


