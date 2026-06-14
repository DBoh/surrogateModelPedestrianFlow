/*!
  \file video_ofiles.c
  \brief functions for video-output
*/
#include <ped.h>

extern VIDEO video;
extern GE *tri;
extern DAT PED;
extern TRI refine;
extern REAL *XY, EpsZero, EpsReg;
extern int NK, NT;
extern REAL PI;
extern MATRIX MAT;
extern GN *node;
extern SIRagent siragent;

/* alle TPs von der Wand wegschieben */
void TrackAvoidWall()
{
  REAL *dummyvec[2], xx, yy, ak, WallV[2], fac=0.1;
  int k1, j, l, i;

  for(l=0 ; l<2 ; l++){
    dummyvec[l] = (REAL *)calloc(NK , sizeof(REAL ));
    for(j=0 ; j<NK ; j++)
      dummyvec[l][j] = node[j].WallNu[l];
  }

  for(i=0 ; i<video.TrackPerson ; i++){
    if(video.TrackInside[i]){
      xx = video.TrackXYPosition[0][i];
      yy = video.TrackXYPosition[1][i];
   
      k1 = search_tri(0,xx,yy,0);
      if(k1>=0 && tri[k1].KindOfBoundary!=2){

  /* Tracks von der Wand wegschieben, nicht beim Ausgang */
	ak = sqrt(area(k1))*2/10; 
	
	for(l=0 ; l<2 ; l++)
	  WallV[l] = uh(k1,xx,yy,dummyvec[l]);

	//	printf("%f %f  -  %f %f\n",xx,yy,WallV[0],WallV[1]);
	xx -= fac*ak*WallV[0];
	yy -= fac*ak*WallV[1];
	video.TrackXYPosition[0][i] = xx;
	video.TrackXYPosition[1][i] = yy;
      } else {
	video.TrackInside[i]=0;
      }
    }
  }
  //  exit(0);
  for(l=0 ; l<2 ; l++)
    free(dummyvec[l]);
  /***********************************/



}

/* wird nur bei TP ausgefuehrt */
void TrackPoint(REAL *rho, REAL *vel[2], int loop)
{
  int l, kt;

  TrackAvoidWall();
  
  for(l=0 ; l<video.TrackPerson ; l++)
    if(video.TrackInside[l])
      video.TrackInside[l] = tracking(&video.TrackXYPosition[0][l],&video.TrackXYPosition[1][l],vel,rho,PED.delta,&kt);
  
  if(PED.numentree)
    AddEntreeTracks(loop);

  
#if SIRA

  sir_agent();

#endif
  
}


/* wird nur bei TP ausgefuehrt */
void AddEntreeTracks(int loop)
{
  REAL tt, EP, xx, yy, infected;
  int le, l, ll, pip, group;
  char mesg[100];
  time_t rtt;
  //  srand((unsigned) time(&rtt));

  tt = loop*PED.delta;
  
  for(le=0 ; le<PED.numentree ; le++){
    // sind noch nicht alle drin?
    EP = PED.EntreePersons[le];   // persons per sec per this door
    group = floor(EP*PED.delta+video.carry[le]);
    if(video.pinsum[le] < PED.MaxEntreePersons[le]-group){

      video.pinloc[le]  = group;
      video.carry[le]  += EP*PED.delta-video.pinloc[le];
      video.pinsum[le] += video.pinloc[le];
      //      printf("pinsum = %d\n",video.pinsum[le]);
      
      // pip persons are new in the crew
      pip = video.pinloc[le];
      if(pip){
	
	for(l=0 ; l<pip ; l++){
	  ll = video.TrackPerson+l+1;
	  if(ll>=video.TrackPersonMax){
	    sprintf(mesg,"%d are too many people (max=%d)",ll,video.TrackPersonMax);
	    ErrorMessage(mesg,"AddEntreeTracks");
	  }
	  xx=(REAL)(rand())/RAND_MAX; yy=(REAL)(rand())/RAND_MAX;
	  xx=PED.entree[0][le]+xx*(PED.entree[2][le]-PED.entree[0][le]);
	  yy=PED.entree[1][le]+yy*(PED.entree[3][le]-PED.entree[1][le]);
	  //	  printf("%f %f\n",xx,yy);
	  video.TrackXYPosition[0][ll] = xx;
	  video.TrackXYPosition[1][ll] = yy;
	  video.TrackInside[ll]        = 1;

	  infected=(REAL)(rand())/RAND_MAX;
	  if(infected<=siragent.PI)
	    video.SIRmode[ll] = 2;
	  else if(infected>=1-siragent.PR)
	    video.SIRmode[ll] = 3;
	  else
	    video.SIRmode[ll] = 0;

	    
	}

	video.TrackPerson += pip;
      }
      
    } 
  }

}


/* wird nur bei TP ausgefuehrt */
void AddEntreeTracks_old(int loop)
{
  REAL tt, EP;
  int le, l, ll;

  for(le=0 ; le<PED.numentree ; le++){
    EP = PED.EntreePersons[le];
    if(video.pinflag[le]){
      video.pinloc[le] = (int)(EP*PED.delta+video.carry[le]);
      video.carry[le] += EP*PED.delta-(double)video.pinloc[le];
      video.pinsum[le] += video.pinloc[le];
      
      
      // pinloc sind die neuen Eintretenden
      if(video.pinsum[le]<EP && video.pinloc[le]){
	for(l=0 ; l<video.pinloc[le] ; l++){
	  
	  video.TrackInside[video.TrackPerson] = 1;
	  tt=min_REAL(max_REAL(RandVal(),0.0),1.0);

	  for(ll=0 ; ll<2 ; ll++)
	    video.TrackXYPosition[ll][video.TrackPerson] = PED.entree[ll][le]*(1.0-tt)
	      +tt*PED.entree[ll+2][le];

	  video.TrackPerson++;
	}
      }
      
      if(video.pinsum[le]>=PED.MaxEntreePersons[le])
	video.pinflag[le]=0;
      
    }
  }
}


/* wird nur bei TP aufgerufen */
int tracking(REAL *x, REAL *y, REAL *vel[2], REAL *rho, REAL delta, int *k)
{
  int l, k1=*k, k2, ll, kk, j;
  REAL d_uh[2], drho[2], tau[2], vtau[2], btau, bvt, rhoh, epsreg=EpsReg, rhoreg, rhok;
  REAL xx=(REAL)*x, yy=(REAL)*y, xneu, yneu, p[2], q[2];
  REAL ttt, sss, diff_fac=1.0, rho_fac=1;
  REAL fdh, fdv, fdr, fda, nx, ny, srr, sra, vstepx, vstepy;
  int l1, l2, aus=0;
  REAL x2, x1, y2, y1, d1, d2, d3, d4;
  REAL phi;
  char cfchk[100];
  REAL srrv1, srrvn, co, si, rneux, rneuy, vrandx, vrandy,noise=0.3;
  int srrv2;
  //  time_t rtt;
  //  int random=1, klebecheck=1, exitcheck=1;
  int random=1, klebecheck=1, exitcheck=1;
  rho_fac=1.;
  //  epsreg = 10;
  
  k1 = search_tri(k1,xx,yy,0);
  if(k1<0) return 0;

  duh(drho,k1,rho);
  rhok = uh(k1,xx,yy,rho);
  for(l=0 ; l<2 ; l++)
    d_uh[l] = uh(k1,xx,yy,vel[l])/PED.CL;


  rhoreg = sqrt(pow(rhok,2.)+pow(epsreg,2.));
  /*
    vstepx = (d_uh[0]-rho_fac*PED.EPS0*drho[0]/rhoreg);
    vstepy = (d_uh[1]-rho_fac*PED.EPS0*drho[1]/rhoreg);
    vstepx = (d_uh[0]-rho_fac*PED.EPS0*drho[0]);
    vstepy = (d_uh[1]-rho_fac*PED.EPS0*drho[1]);
    vstepx = (d_uh[0]-rho_fac*PED.EPS0*drho[0]*rhok);
    vstepy = (d_uh[1]-rho_fac*PED.EPS0*drho[1]*rhok);
  */
    vstepx = (d_uh[0]-rho_fac*PED.EPS0*drho[0]);
    vstepy = (d_uh[1]-rho_fac*PED.EPS0*drho[1]);

  
  // rauschen auf die Bewegung bringen
  if(random){
      vrandx = vstepx; vrandy = vstepy;
      srrv1 = PI*noise*((REAL)(rand())/RAND_MAX-0.5);

      co = cos(srrv1); si = sin(srrv1);
      
      vstepx = co*vrandx - si*vrandy;
      vstepy = si*vrandx + co*vrandy;
  }
  
  xneu = xx+delta*vstepx;
  yneu = yy+delta*vstepy;
  

  k2 = search_tri(k1,xneu,yneu,0);

  // outside_____________________________________________________-
  if(k2<0){
  
    for(l=0 ; l<PED.numexit; l++){
      // Verlaesst das Gebiet durch den Ausgang?
      if(SegSchnitt(xx,yy,xneu,yneu,&ttt,PED.exit[0][l],PED.exit[1][l],
                    PED.exit[2][l],PED.exit[3][l],&sss)) return 0;
      // nahe am Ausgang rauswerfen
      if(fabs(DistPointLineSegment(xneu,yneu,PED.exit[0][l],PED.exit[1][l],
				   PED.exit[2][l],PED.exit[3][l]))<gridwidth_loc(k1)/10) return 0;
    }

    // draussen aber nicht durch den Ausgang
    if(SegBoundarySchnitt(xx,yy,xneu,yneu,&kk,&ll)){

      // Slippen
      btau = 0.;
      for(l=0 ; l<2 ; l++){
	tau[l] = XY[2*tri[kk].VNR[(ll+2)%3]+l]-XY[2*tri[kk].VNR[(ll+1)%3]+l];
	btau += tau[l]*tau[l];
      }

      vtau[0] = xneu-xx; vtau[1] = yneu-yy; 

      //      printf("%.2e ",sqrt(vtau[0]*vtau[0]+vtau[1]*vtau[1]));
      
      bvt = (vtau[0]*tau[0]+vtau[1]*tau[1])/btau;
      
      for(l=0 ; l<2 ; l++)
	vtau[l] = bvt*tau[l];

      //      printf(" %.2e\n",sqrt(vtau[0]*vtau[0]+vtau[1]*vtau[1]));


      xneu = xx+vtau[0];
      yneu = yy+vtau[1];

      if(kk==search_tri(kk,xneu,yneu,0)){
	*x = xneu; *y = yneu; *k = kk;
#if MSG==2	
	printf("gerettet: %f %f\n",xneu,yneu);
#endif
	return 1;
      }
      // PArtikel verloren!!!
      return 0;
      
    } else  // PArtikel verloren!!!
      return 0;
    
    // inside_____________________________________________________-
  } else {


    
    *x = xneu;
    *y = yneu;

    return 1;

  }

  
}







