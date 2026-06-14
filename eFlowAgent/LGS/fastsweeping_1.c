/*!
  \file equations.c
  \brief several equations
*/
#include <ped.h>

extern TRI refine;
extern SOLV solver;
extern REAL EpsZero, EpsReg;
extern DAT PED;
extern MATRIX MAT;
extern SUBDOM SDCV;
extern GN *node;
extern GE *tri;
extern Basis phi;
extern QUADRATUR QUAD;
extern int NT, NK, *nach, bandwidth;
extern REAL *XY, PI;
extern NEUTRALS ZO;


//Aufruf: HelmholtzEquation(ped_time,rho,phi,one,zero,PED_Helm_bound2vec);
//
void FastSweeping(int loop, REAL *rho, REAL *phi, void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *))
{
  int i, k, iloops, j, kj, i1, i2, i3;
  int nRef = 6;
  int refPoints[nRef];
  int temp;
  int *order;
  REAL *distance;
  REAL *f;
  REAL x1, fa, fb, fc, len,  a, b, c,  alpha, beta, gamma;
  REAL y1, theta, x2, H, h, y2, x3, y3, xmin;
  
  f        = (REAL *)calloc(NK , sizeof(REAL ));
  distance = (REAL *)calloc(NK , sizeof(REAL ));
  order    = (int *) calloc(NK , sizeof(int ));

  
  FD_Vec(f,rho,0);
  
  for(i=0 ;i<NK ;i++)
    {
      phi[i]=100;
      if(node[i].id<0 && ChkExit(i))
      	phi[i] = 0.; //PED.ExitAttraction[ne-1]*(1-PED.VMin)+PED.VMin;
    }
  
  for(k=0; k<nRef; k++)
      refPoints[k]= rand()%NK;

  refPoints[1]=1;
  refPoints[nRef] = NK;
  
  for(iloops=0;iloops < 100;iloops++)
    {
      
      //      printf("Loop:%i\n",iloops);
      for (k=0 ; k<NK ; k++)
	{
	  distance[k] = sqrt(pow(XY[2*k]-XY[2*refPoints[1]],2.0)+pow(XY[2*k+1]-XY[2*refPoints[1]+1],2.0));
	  order[k]=k;
	}
      
      for (i = 0; i < NK; ++i){
	for (j = i + 1; j < NK; ++j){
	  if (distance[i] > distance[j]){
	    temp=order[i];
	    order[i] = order[j];
	    order[j] = temp;
	  }
	}
      }
      
      
      for(i=0 ; i<NK ; i++){
	kj = node[order[i]].NEIGH_TRI[0];
	
	for(j=1 ; j<= kj ; j++){
	  k = node[i].NEIGH_TRI[j];
	  i3 = tri[k].VNR[0];
	  x3 = XY[2*i3];
	  y3=XY[2*i3+1];
	  
	  i2=tri[k].VNR[1];
	  x2=XY[2*i2];
	  y2=XY[2*i2+1];
	  
	  i1=tri[k].VNR[2];
	  x1=XY[2*i1];
	  y1=XY[2*i1+1];
	  
	  len = abs(phi[i1]-phi[i2]);
	  fc = 1./(f[i3]*f[i3]);
	  
	  a=sqrt(pow(x2-x3,2)+pow(y2-y3,2));
	  b=sqrt(pow(x1-x3,2)+pow(y1-y3,2));
	  c=sqrt(pow(x2-x1,2)+pow(y2-y1,2));
	  
	  alpha = acos(((x2-x1)*(x1-x3)+(y2-y1)*(y1-y3))/b/c);
	  alpha = fmin(alpha,M_PI-alpha);
	  beta = acos(((x2-x3)*(x2-x1)+(y2-y3)*(y2-y1))/a/c);
	  
	  beta = fmin(beta,M_PI-beta);
	  
	  if (len <= c*fc)
	    {
	      theta = asin(abs(phi[i1]-phi[i2])/c/fc);
	      
	      if ((fmax(0,alpha-M_PI/2)<=theta & theta <=M_PI/2-beta) || (alpha-M_PI/2 <= theta && theta <=min(0,M_PI/2-beta)))
		{
		  h=a*sin(beta-theta);
		  H=b*sin(alpha+theta);
		  phi[i3]=fmin(phi[i3],0.5*(h*fc+phi[i2])+0.5*(H*fc+phi[i1]));
		  
		}
	      else
		{
		  phi[i3]=fmin(phi[i3],fmin(phi[i1]+b*fc,phi[i2]+a*fc));
		  
		}
	    }else
	    {
	      phi[i3]=fmin(phi[i3],fmin(phi[i1]+b*fc,phi[i2]+a*fc));
	      
	    }
	  
	}
	
	
      }
      
      for (k=0 ; k<NK ; k++)
	{
	  distance[k] = sqrt(pow(XY[2*k]-XY[2*refPoints[2]],2.0)+pow(XY[2*k+1]-XY[2*refPoints[2]+1],2.0));
	  order[k]=k;
	}
      
      for (i = 0; i < NK; ++i){
	for (j = i + 1; j < NK; ++j){
	  if (distance[i] > distance[j]){
	    temp=order[i];
	    order[i] = order[j];
	    order[j] = temp;
	  }
	}
      }
      
      for(i=0 ; i<NK ; i++){
	kj = node[order[NK-i-1]].NEIGH_TRI[0];
	
	for(j=1 ; j<= kj ; j++){
	  k = node[i].NEIGH_TRI[j];
	  i1 = tri[k].VNR[0];
	  x1 = XY[2*i1];
	  y1=XY[2*i1+1];
	  
	  i2=tri[k].VNR[1];
	  x2=XY[2*i2];
	  y2=XY[2*i2+1];
	  
	  i3=tri[k].VNR[2];
	  x3=XY[2*i3];
	  y3=XY[2*i3+1];
	  
	  len = abs(phi[i1]-phi[i2]);
	  fa = 1./(f[i1]*f[i1]);
	  fb = 1./(f[i2]*f[i2]);
	  fc = 1./(f[i3]*f[i3]);
	  
	  a=sqrt(pow(x2-x3,2)+pow(y2-y3,2));
	  b=sqrt(pow(x1-x3,2)+pow(y1-y3,2));
	  c=sqrt(pow(x2-x1,2)+pow(y2-y1,2));
	  
	  alpha = acos(((x2-x1)*(x1-x3)+(y2-y1)*(y1-y3))/b/c);
	  alpha = fmin(alpha,M_PI-alpha);
	  beta = acos(((x2-x3)*(x2-x1)+(y2-y3)*(y2-y1))/a/c);
	  beta = fmin(beta,M_PI-beta);
	  gamma = acos(((x2-x3)*(x1-x3)+(y2-y3)*(y1-y3))/b/a);
	  gamma = fmin(gamma,M_PI-gamma);
	  
	  
	  if (len <= c*fc)
	    {
	      theta = asin(abs(phi[i1]-phi[i2])/c/fc);
	      
	      if ((fmax(0,alpha-M_PI/2)<=theta & theta <=M_PI/2-beta) || (alpha-M_PI/2 <= theta && theta <=fmin(0,M_PI/2-beta)))
		{
		  h=a*sin(beta-theta);
		  H=b*sin(alpha+theta);
		  
		  phi[i3]=fmin(phi[i3],0.5*(h*fc+phi[i2])+0.5*(H*fc+phi[i1]));
		}
	      else
		{
		  phi[i3]=fmin(phi[i3],fmin(phi[i1]+b*fc,phi[i2]+a*fc));
		  
		}
	    }else
	    {
	      phi[i3]=fmin(phi[i3],fmin(phi[i1]+b*fc,phi[i2]+a*fc));
	      
	      
	    }
	  
	}
	//	printf("phi=%f\n",phi[i3]);
	
      }
      //      printf("iloop=%i\n",iloops);
    }
  //  printf("LOOP=%i\n",loop);
  
  //  SolHelm2Eikonal(v,phi);
  
#if PLOT==2
  
  octaveplot(phi,"FSphi",0.,-1.,loop);
  PlotBDphi(loop,phi,rho,0);
  
#endif

  free(distance);
  free(order);
  free(f);
}

