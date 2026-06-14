/*!
  \file pFlowSIM.c

  \brief Pedestrian Flow Simulation
*/
#include <time.h>
#include <ped.h>
#include <externparam.h>
#include <tools.h>

REAL Eps=1.0e-12;

void GetTriPoints(REAL *TriPoint[2], int numTP, int flag);
int FindTriangle(REAL, REAL);
REAL area_local(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3){return fabs((x3-x1)*(y2-y1)-(x2-x1)*(y3-y1))*0.5;}



int main(int argc, char *argv[])
{
  int i, k=0, l;
  REAL *rho, *unu, *v, *vel[2];
  time_t ttt;

  srand((unsigned) time(&ttt));
  sprintf(femlog,"%s.log",argv[0]);

  //-------------------------------------------------------------
  //Initialisierung: Data

  standardvalues();
  ReadFEMData();
  ReadPEDParam();

  //-------------------------------------------------------------
  //Initialisierung: Grid

  assign();
  AssignGrid(PED_bound,&numdOmega);

  int numTP, flag, count;
  REAL *TriPoint[2];

  numTP=NT;
  for(l=0 ; l<2 ; l++)
    TriPoint[l] = (REAL *)calloc(numTP , sizeof(REAL ));

  /* Punkte setzen */
  /* flag in {1} bestimmt Arten von gesetzten Punkten */
  flag = 1;
  GetTriPoints(TriPoint,numTP,flag);

  /* Dreiecke finden */
  /* flag in {0} schaltet Suche aus 
     flag in {1,2} waehlt aus verschiedenen Methoden */
  flag = 2;
  if(flag){
    switch(flag)
      {
	case 1:
	  count = 0;
	  for(i=0 ; i<numTP ; i++){
	    k = FindTriangle(TriPoint[0][i],TriPoint[1][i]);
	    if(k>0) count++;
	  }
	  printf("%d Dreiecke von %d gefunden\n",count,numTP);
	  break;
	case 2:
	  count = 0;
	  for(i=0 ; i<numTP ; i++){
	    k = search_tri(k,TriPoint[0][i],TriPoint[1][i],0);
	    if(k>0) count++;
	  }
	  printf("%d Dreiecke von %d gefunden\n",count,numTP);
	  break;
	  default:
	    break;
      }
  }

  
  /* cleaning up */
  FreeParam();


  return 0;
}


int FindTriangle(REAL x, REAL y)
{
  REAL a1, a2, a3, aT;
  REAL x1, x2, x3, y1, y2, y3;
  int i1, i2, i3, out=0, k;

  for(k=0 ; k<NT ; k++){
    i1 = tri[k].VNR[0];
    i2 = tri[k].VNR[1];
    i3 = tri[k].VNR[2];
    x1 = XY[2*i1];
    y1 = XY[2*i1+1];
    x2 = XY[2*i2];
    y2 = XY[2*i2+1];
    x3 = XY[2*i3];
    y3 = XY[2*i3+1];
    
    aT = area_local(x1,y1,x2,y2,x3,y3);
    a1 = area_local(x1,y1,x2,y2,x,y);
    a2 = area_local(x3,y3,x2,y2,x,y);
    a3 = area_local(x1,y1,x3,y3,x,y);
    if(fabs(aT-(a1+a2+a3))<Eps) return 1;
  }
}


void GetTriPoints(REAL *TriPoint[2], int numTP, int flag)
{
  int k, l;

  switch(flag){
  case 1:
    // Points als Mittelpunkte eines Dreiecks
    for(k=0 ; k<numTP ; k++){
      for(l=0 ; l<3 ; l++){
	TriPoint[0][k] += XY[2*tri[k].VNR[l]];
	TriPoint[1][k] += XY[2*tri[k].VNR[l]+1];
      }
      for(l=0 ; l<2 ; l++)
	TriPoint[l][k] /= 3.;
    }
    break;
  default:
    break;
  }
}
