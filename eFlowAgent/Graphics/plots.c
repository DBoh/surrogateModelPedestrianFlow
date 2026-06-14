#include <ped.h>

extern VIDEO video;
extern DAT PED;
extern REAL GridEpsZero;
extern GN *node;
extern REAL *XY;
extern int NT, NK;
extern GE *tri;
extern TRI refine;

// Des
void PED_bound()
{
  REAL X, Y, X1, X2, Y1, Y2, XX, YY;
  REAL eps=min_REAL(1.e-02,pow(refine.gridwidth,2.));
  REAL dist1, dist2, dist3;
  int k, j, je, jj, jjj, l, qb;

  for(k=0 ; k<NT ; k++){
    for(j=0 ; j<3 ; j++){
      if(tri[k].NEIGH[j]<0){
	jj  = tri[k].VNR[(j+1)%3];
	jjj = tri[k].VNR[(j+2)%3];
	node[jj].id = -2;
	node[jjj].id = -2;


	X  = XY[2*jj]; Y  = XY[2*jj+1];
	XX = XY[2*jjj]; YY = XY[2*jjj+1];
	
	// (X,Y) auf Eingaenge pruefen
	for(l=0 ; l<PED.numentree ; l++){
	  X1 = PED.entree[0][l]; Y1=PED.entree[1][l];
	  X2 = PED.entree[2][l]; Y2=PED.entree[3][l];
	  
	  if(fabs(sqrt(pow(Y-Y1,2.0)+pow(X-X1,2.0))+
		  sqrt(pow(Y-Y2,2.0)+pow(X-X2,2.0))-
		  sqrt(pow(Y1-Y2,2.0)+pow(X1-X2,2.0)))<eps)
	    node[jj].id = -2*(l+2);

	  if(fabs(sqrt(pow(YY-Y1,2.0)+pow(XX-X1,2.0))+
		  sqrt(pow(YY-Y2,2.0)+pow(XX-X2,2.0))-
		  sqrt(pow(Y1-Y2,2.0)+pow(X1-X2,2.0)))<eps)
	    node[jjj].id = -2*(l+2);
	  //	  break;
	}
      

	// (X,Y) auf Ausgang pruefen
	for(l=0 ; l<PED.numexit ; l++){
	  X1 = PED.exit[0][l]; Y1=PED.exit[1][l];
	  X2 = PED.exit[2][l]; Y2=PED.exit[3][l];
	  
	  //	  qb = QuaderBound(X1,Y1,X2,Y2,X,Y);
	  if(fabs(sqrt(pow(Y-Y1,2.0)+pow(X-X1,2.0))+
		  sqrt(pow(Y-Y2,2.0)+pow(X-X2,2.0))-
		  sqrt(pow(Y1-Y2,2.0)+pow(X1-X2,2.0)))<eps)
	    /*	  if(QuaderBound(X1,Y1,X2,Y2,X,Y)){*/
	    node[jj].id = -(2*l+3);
	    
	  if(fabs(sqrt(pow(YY-Y1,2.0)+pow(XX-X1,2.0))+
		  sqrt(pow(YY-Y2,2.0)+pow(XX-X2,2.0))-
		  sqrt(pow(Y1-Y2,2.0)+pow(X1-X2,2.0)))<eps)
	    /*	  if(QuaderBound(X1,Y1,X2,Y2,X,Y)){*/
	    node[jjj].id = -(2*l+3);
	  //	    break;
	}
      }

    }
  }


}

