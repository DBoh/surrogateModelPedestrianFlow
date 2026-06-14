/*!
  \file sir.c
  \brief sir-functions for agents
*/
#include <ped.h>


extern VIDEO video;
extern SIRagent siragent;
extern DAT PED;
extern REAL *XY;
extern GE *tri;

/*
extern TRI refine;
extern REAL EpsZero, EpsReg;
extern REAL PI;
extern MATRIX MAT;
extern GN *node;
extern SUBDOM RhoIni;
*/

void sir_agent_init()
{
  int i, numIinit, numRinit;

  numIinit = round(PED.Pinit*siragent.PI);
  numIinit = min(PED.Pinit,max(1,numIinit));

  numRinit = round(PED.Pinit*siragent.PR);
  numRinit = min(PED.Pinit,max(1,numRinit));

  if(numRinit+numIinit>PED.Pinit)
    {
      if(numRinit>numIinit)
	numRinit = PED.Pinit-numIinit;
      else
	numIinit = PED.Pinit-numRinit;
    }
  
  for(i=0; i<numIinit ; i++)
    video.SIRmode[i] = 2;

  for(i=numIinit ; i<numIinit+numRinit ; i++)
    video.SIRmode[i] = 3;


  siragent.SEIRMonitoring[0] = PED.Pinit-(numIinit+numRinit);
  siragent.SEIRMonitoring[1] = 0;
  siragent.SEIRMonitoring[2] = numIinit;
  siragent.SEIRMonitoring[3] = numRinit;
}



void sir_agent()
{
  int i, j, k, l;
  REAL diff, xdt, ydt, rn, x1, y1, x2, y2;

  // Parameter
  REAL CD;  // critical distance
  REAL IR;  // infection rate
  REAL RT;  // ressistance time

  CD = siragent.CD;
  IR = siragent.IR;
  RT = siragent.RT;

  for(i=0 ; i<video.TrackPerson ; i++)
    {
      // only if i is infectious
      if(video.TrackInside[i] && video.SIRmode[i]==2)
	{

	  x1 = video.TrackXYPosition[0][i];
	  y1 = video.TrackXYPosition[1][i];
		  
	  for(j=0 ; j<video.TrackPerson ; j++)
	    {
	      // only if j is (re-) attachable
	      if(video.TrackInside[j] && i!=j && !video.SIRmode[j])
		{

		  x2 = video.TrackXYPosition[0][j];
		  y2 = video.TrackXYPosition[1][j];
		  
		  xdt = x1-x2; 
		  ydt = y1-y2; 
		  diff = xdt*xdt+ydt*ydt;

		  if(!SegBoundarySchnitt(x1,y1,x2,y2,&k,&l))
		    {
		      
		      if(diff<CD*CD)
			{
			  rn = (REAL)(rand())/RAND_MAX;
			  
			  if(rn<=IR*PED.delta/RT)
			    {
			      video.SIRmode[j] = 1;
			      siragent.SEIRMonitoring[0] -= 1;
			      siragent.SEIRMonitoring[1] += 1;
			    }
			
			}
		    }
		}
	    }
	}
    }
}
		  
	    
	    
		  
		  
		  
      
		  
    

