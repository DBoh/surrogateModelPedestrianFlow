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

//s. Qian, J.; Zhang, Y.; Zhao, H.: Fast Sweeping Methods on Triangular Meshes. SIAM J. Numer. Anal. Vol.45, No.1, pp.83-107, 2007.

//Aufruf: HelmholtzEquation(ped_time,rho,phi,one,zero,PED_Helm_bound2vec);


void FastSweeping(int loop, REAL *rho, REAL *phi, void (*dbound)(REAL , REAL *, int ), void (*amul_bd)(REAL *, REAL *, REAL *))
{
  int i, k, iloops, j, kj, i1, i2, i3,l;
  int nRef =6;
  int nloops = 1000;
  int refPoints[nRef];
  int temp;
  int* order;
  REAL *distance;
  REAL *f;
  REAL *phiold;
  REAL x1, fa, fb, fc, len,  a, b, c,  alpha, beta, gamma,error;
  REAL y1, theta, x2, H, h, y2, x3, y3, xmin;
  REAL epsilon = 0.0001;
  
  f        = (REAL *)calloc(NK , sizeof(REAL ));
  distance = (REAL *)calloc(NK , sizeof(REAL ));
  phiold = (REAL *)calloc(NK , sizeof(REAL ));
  order = (int *)calloc(nRef*NK,sizeof(REAL));
  
  
  
  
  
  // initialization right side
  FD_Vec(f,rho,0);
  
  // initialization phi
  k=0;
  for(i=0 ;i<NK ;i++)
    {
      phi[i]=100;
      if(node[i].id<0 && ChkExit(i)){
      	phi[i] = 0.; //PED.ExitAttraction[ne-1]*(1-PED.VMin)+PED.VMin;
	//  	refPoints[k]=i;
	//	k++;
      }
    }
  
  // set arbitrary reference points
  for(k=0; k<nRef; k++)
    {refPoints[k]= rand()%NK;}
  
  //refPoints[0]=0;
  //refPoints[nRef] = NK;
  
  // sort nodes w.r.t. distance to reference points
  for (l=0; l < nRef;l++){
    for (k=0 ; k<NK ; k++)
      {
	distance[k] = sqrt(pow(XY[2*k]-XY[2*refPoints[l]],2.0)+pow(XY[2*k+1]-XY[2*refPoints[l]+1],2.0));
	order[l*NK+k]=k;
      }
    
    for (i = 0; i < NK-1; i++){
      for (j = i; j < NK-1; j++){
	
	if (distance[order[l*NK+j]] > distance[order[l*NK+j+1]]){
	  temp=order[l*NK+j];
	  order[l*NK+j] = order[l*NK+j+1];
	  order[l*NK+j+1] = temp;
	  
	}
      }
    }
    
  }
  for(i =0;i<NK;i++){phiold[i]=phi[i];}
  // start loop
  for(iloops=0;iloops < nloops;iloops++)
    {
      for(l=0;l<nRef;l++){
	
	for(i=0 ; i<NK ; i++){
	  kj = node[order[l*NK+i]].NEIGH_TRI[0];
	  
	  for(j=1 ; j<= kj ; j++){
	    k = node[order[l*NK+i]].NEIGH_TRI[j];
	    if(tri[k].VNR[0] == order[l*NK+i]){
	      i3 =order[l*NK+i];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[1];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[2];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    
	    if(tri[k].VNR[1] == order[l*NK+i]){
	      i3 =order[l*NK+i];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[2];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[0];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    if(tri[k].VNR[2] == order[l*NK+i]){
	      i3 =order[l*NK+i];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[0];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[1];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    len = fabs(phi[i1]-phi[i2]);
	    
	    fc = 1./(f[i3]);
	    
	    a=sqrt(pow(x2-x3,2)+pow(y2-y3,2));
	    b=sqrt(pow(x1-x3,2)+pow(y1-y3,2));
	    c=sqrt(pow(x2-x1,2)+pow(y2-y1,2));
	    
	    alpha = acos(((x2-x1)*(x1-x3)+(y2-y1)*(y1-y3))/b/c);
	    //  alpha = fmin(alpha,M_PI-alpha);
	    beta = acos(((x2-x3)*(x2-x1)+(y2-y3)*(y2-y1))/a/c);
	    
	    //beta = fmin(beta,M_PI-beta);
	    //printf("phi before:%f\n",phi[i3]);
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
	
	
	
	for(i=0 ; i<NK ; i++){
	  kj = node[order[(NK-i-1)+l*NK]].NEIGH_TRI[0];
	  
	  for(j=1 ; j<= kj ; j++){
	    k = node[order[NK-i-l+l*NK]].NEIGH_TRI[j];
	    if(tri[k].VNR[0] == order[NK-i-1+l*NK]){
	      i3 =order[NK-i-1+l*NK];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[1];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[2];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    
	    if(tri[k].VNR[1] == order[NK-i-1+l*NK]){
	      i3 =order[NK-i-1+l*NK];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[2];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[0];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    if(tri[k].VNR[2] == order[NK-i-1+l*NK]){
	      i3 =order[NK-i-1+l*NK];// tri[k].VNR[0];
	      x3 = XY[2*i3];
	      y3=XY[2*i3+1];
	      
	      i2=tri[k].VNR[0];
	      x2=XY[2*i2];
	      y2=XY[2*i2+1];
	      
	      i1=tri[k].VNR[1];
	      x1=XY[2*i1];
	      y1=XY[2*i1+1];}
	    
	    len = fabs(phi[i1]-phi[i2]);
	    
	    fc = 1./(f[i3]);
	    
	    a=sqrt(pow(x2-x3,2)+pow(y2-y3,2));
	    b=sqrt(pow(x1-x3,2)+pow(y1-y3,2));
	    c=sqrt(pow(x2-x1,2)+pow(y2-y1,2));
	    
	    alpha = acos(((x2-x1)*(x1-x3)+(y2-y1)*(y1-y3))/b/c);
	    //  alpha = fmin(alpha,M_PI-alpha);
	    beta = acos(((x2-x3)*(x2-x1)+(y2-y3)*(y2-y1))/a/c);
	    //beta = fmin(beta,M_PI-beta);
	    
	    
	    
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
	}
      }
      
      
      field_cp(phiold, 1, phi, -1, phiold, 0, NK);
      error = sqrt(scalpr(phiold,phiold,NK));
      //      printf("error:%f\n",error);
      if(error < epsilon){ break;}
      for(i =0;i<NK;i++){phiold[i]=phi[i];}
      //      printf("LOOP=%i\n",iloops);
    }
  
  
  //  SolHelm2Eikonal(v,phi);
  
#if PLOT==2
  
  octaveplot(phi,"FSPhi",0.,-1.,loop);
  octaveplot(rho,"FSRho",0.,-1.,loop);
  octaveplot(f,"FSf",0.,-1.,loop);
  
#endif
  
  free(distance);
  free(f);
  free(phiold);
  
}

