#include <ped.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int NK, NT;
extern REAL *XY;
extern GE *tri;
extern POLY DomainPoly;

/* Precision */
/* If SINGLE is defined when triangle.o is compiled, it should also be       */
/*   defined here.  If not, it should not be defined here.                   */

/* #define SINGLE */

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */



void meshgen(){
  struct triangulateio in, mid, out, vorout;
  int i, k, l;

  if(DomainPoly.NumK) { // wenn ein Polygonzug angegeben wurde


    /* Polygonzug skalieren fuer feinere Granularitaet  */

    REAL Pmin[2], Pxmax, Pscal, aaa=1;

    //REAL *Spoly;
    //Spoly = (REAL *)calloc(2*DomainPoly.NumK , sizeof(REAL ));

    Pmin[0]=DomainPoly.XY[0];
    Pmin[1]=DomainPoly.XY[1]; Pxmax=Pmin[0];
    for(i=1 ; i<DomainPoly.NumK ; i++){
      for(l=0 ; l<2 ; l++)
	if(DomainPoly.XY[2*i+l]<Pmin[l]) Pmin[l] = DomainPoly.XY[2*i+l];

      if(DomainPoly.XY[2*i]>Pxmax) Pxmax = DomainPoly.XY[2*i];
    }

    Pscal = fabs(Pxmax-Pmin[0]);

    for(i=0 ; i<DomainPoly.NumK ; i++)
      for(l=0 ; l<2 ; l++)
	DomainPoly.XY[2*i+l] = (DomainPoly.XY[2*i+l]-Pmin[l])*aaa/Pscal;

    if(DomainPoly.NumH)
      for(i=0 ; i<DomainPoly.NumH ; i++)
	for(l=0 ; l<2 ; l++)
	  DomainPoly.HOLES[2*i+l] = (DomainPoly.HOLES[2*i+l]-Pmin[l])*aaa/Pscal;

    in.numberofpoints = DomainPoly.NumK; //numPoints;
    in.numberofpointattributes = 0;

    in.pointlist = DomainPoly.XY; //pointList;

    in.pointmarkerlist = NULL;

    // what are regions?
    in.numberofsegments = DomainPoly.NumS; //numberOfSegments;
    in.segmentlist = DomainPoly.SEG; //segmentlist;
    in.segmentmarkerlist = NULL;
    in.numberofholes = DomainPoly.NumH; //numHoles;
    in.numberofregions = 0;

    in.holelist = DomainPoly.HOLES; //holelist;

    /* Make necessary initializations so that Triangle can return a */
    /*   triangulation in `mid' and a voronoi diagram in `vorout'.  */

    mid.pointlist = (REAL *)NULL; /* Not needed if -N switch used. */
    /* Not needed if -N switch used or number of point attributes is zero: */
    mid.pointattributelist = (REAL *)NULL;
    mid.pointmarkerlist = (int *)NULL; /* Not needed if -N or -B switch used. */
    mid.trianglelist = (int *)NULL;    /* Not needed if -E switch used. */
    /* Not needed if -E switch used or number of triangle attributes is zero: */
    mid.triangleattributelist = (REAL *)NULL;
    mid.neighborlist = (int *)NULL; /* Needed only if -n switch used. */
    /* Needed only if segments are output (-p or -c) and -P not used: */
    mid.segmentlist = (int *)NULL;
    /* Needed only if segments are output (-p or -c) and -P and -B not used: */
    mid.segmentmarkerlist = (int *)NULL;
    mid.edgelist = (int *)NULL;       /* Needed only if -e switch used. */
    mid.edgemarkerlist = (int *)NULL; /* Needed if -e used and -B not used. */

    vorout.pointlist = (REAL *)NULL; /* Needed only if -v switch used. */
    /* Needed only if -v switch used and number of attributes is not zero: */
    vorout.pointattributelist = (REAL *)NULL;
    vorout.edgelist = (int *)NULL;  /* Needed only if -v switch used. */
    vorout.normlist = (REAL *)NULL; /* Needed only if -v switch used. */

    /* Triangulate the points.  Switches are chosen to read and write a  */
    /*   PSLG (p), preserve the convex hull (c), number everything from  */
    /*   zero (z), assign a regional attribute to each element (A), and  */
    /*   produce an edge list (e), a Voronoi diagram (v), and a triangle */
    /*   neighbor list (n).                                              */

    char switches[20];

    sprintf(switches,"pQDza%.7f",DomainPoly.Granu);
       printf("%s\n",switches);
    //exit(0);
    triangulate(switches, &in, &mid, &vorout);

    NK = mid.numberofpoints;
    NT = mid.numberoftriangles;

    XY = (REAL *)malloc(2*NK*sizeof(REAL));
    tri = (GE *)malloc(NT*sizeof(GE));

    for(i=0 ; i<2*NK ; i++)
      XY[i] = mid.pointlist[i];

    i=0;
    for(k=0 ; k<NT ; k++)
      for(l=0 ; l<3 ; l++)
	tri[k].VNR[l] = mid.trianglelist[i++];


    /* Ruecktransformation */
    for(i=0 ; i<NK ; i++)
      for(l=0 ; l<2 ; l++)
	XY[2*i+l] = XY[2*i+l]*Pscal/aaa+Pmin[l];


  }
}


