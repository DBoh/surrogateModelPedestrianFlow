#pragma once
#include <std.h>


typedef struct {
  REAL Mass;    // Masse=Anzahl Personen im Raum
  REAL *Exits;  // Anzahl Personen, die den Raum durch Tuer i verlassen haben
  REAL Exit;    // Anzahl Personen (kumulativ), die den Raum verlassen haben
  REAL Entree;  // Anzahl Personen (kumulativ), die den Raum betreten haben
  REAL Rhomin, Rhomax;
  REAL S_EIR, S_E_IR, SE_I_R, SEI_R; // SEIR-Entwichlungen
  int MS;
  REAL *MSpeople;
  char Message[200];
} Monitoring;


typedef struct{
  REAL CL, CT, CP, CV, *CVsd;
  int FD, FDnk, eFlowfile;
  char FDname[100], ConfigFile[100];
  REAL *FundamentalDiagram[2], *dFundamentalDiagram[2];
  REAL FD_SpecificPower;
  REAL EPS0, EPS1, EPS2;
  //--------------------------- init data
  int Pinit, Prand;
  //--------------------------- rhs door
  REAL *EntreePersons, *MaxEntreePersons, *pFlowDoor;
  REAL *DensityDoor;  // *RhoInflow, 
  REAL Shoulder; // Schulterbreite
  REAL *MaxTimeDoor;
  REAL *entree[4], *exit[4];
  REAL *stage[4];
  int numentree, numexit, numstage;
  REAL *ExitAttraction, VMin;
  //---------------------------
  REAL MaxTime;
  //--------------------------- Monitoring
  int MS; //, *KOF_MS;
  REAL *MSx[4];
  //  REAL *Mval[2];
  REAL MassMax;
    //--------------------------- attractors
  int AS;
  REAL *ASx[7];  //x1,y1,x2,y2,attraction,distance of attr.
  REAL *ASphi;   // randmized phases for periodic attracors
  REAL *ARMask;
  //---------------------------
  REAL delta, empty;
  REAL plotfreq;
} DAT;


typedef struct{
  int NumK, NumS, NumH;
  int *SEG;
  int *PolyNo;         // Zuweisung Knotennr zu Polygonnr
  int CCW;             // = 1: counterclockwise = true (orientation)
  REAL *XY, *HOLES;
  REAL Area, Granu;
} POLY;



typedef struct{
  int NumSubdom, PKmax;
  int *KnotToSubdom;
  REAL *Kvec;
  REAL *Tvec;
  POLY *P;
} SUBDOM;




typedef struct{
  int *Q;
  int Qnum;
} QUADRA;


typedef struct{
  REAL *NKones, *NKzeros;
  REAL *NTones, *NTzeros;
  REAL *Entreeones, *Exitones;
} NEUTRALS;

typedef struct{
  REAL *Mass, *Stiff;
  REAL *MassExit, *MassEntree;
  REAL *KonvExit, *KonvEntree;
} MATRIX;

// triangle properties
typedef struct{
  int NEIGH[3];             // neighborhood
  int VNR[6];               // global Vertex-No
  int RE;                   // refinement edge
  int mark;                 // Markierung zur Verfeinerung
  int BoundTri[4];          // amount boundary nodes, no. b-node,...
  int KindOfBoundary;       // [0,1,2,3] = [inner, bound, exit, entree]
} GE;

typedef struct{
  int *NEIGH_NODE;          // neighbornodes
  int *NEIGH_TRI;           // neighbortriangles
  int pointmat;             // pointer to the diagonal element of matrices
  int id;                   // type of node [j,-1,-2,-(2*l+2),-(2*l+3)] = [inner,neumann/dirichlet, boundary,entree,exit], l=no. of entree, resp. exit 
  int neumann;
  REAL WallNu[2];
  REAL EntreeNu[2];
} GN;

// refine
typedef struct{
  int lg, ll, mode;               // global and local refinements
  char gridfile[200];       // file with makrotriangulation
  char subdomainFD[200], subdomainRhoini[200];  
  REAL boundx_ll, boundx_ur, boundy_ll, boundy_ur;
  REAL gridwidth, gridmin, gridmax, scale;
  REAL domarea;
  // Randdreiecke, Randkanten 
  int numdOmega, *dOmega[4], numdOmegaRing, *dOmegaList, *dOmegaStartlist;
} TRI;

typedef struct{
  int *Geom;
  int mom, child;
  int Level;
} ShortestPath;


typedef struct{
  REAL st_1d[2], w_1d[2];
  REAL st_2d[3][2], w_2d[3];
} QUADRATUR;


//=======================================================================


typedef struct{
  int KOI;                  // welche Sorte Infos werden ausgegeben: (0=none, 1=errors, 2=errors&warnings, 3=just infos, a=all)
  int IL;                   // Infotiefe [0:10]
  int edge, corr;           // plot Kantenbild, Korrekturbild [0:1]
} INFO;

typedef struct {
  int GnuPlot;
  int Warning;
  int Check;
  int Error;
} INFOS;

typedef struct{
  int DOFloc;
  int NOsamp;
  int DEG;
  REAL value[6][7];
  REAL gradx[6][2][3];
  REAL conv_mass[6][6][6], mass[6][6], stiff[6][6];
} Basis;

typedef struct{
  int mode, itmax, precon;
  REAL akrit;
} SOLV;

typedef struct{
  int bound;
  REAL alpha, beta, gamma, delta;
} System;

typedef struct {
  int Radius;
  REAL *TrackXYPosition[2];
  int TrackPerson, TrackPersonFrequenz, TrackPersonMax, *TrackInside;
  REAL *Rhoh;
  REAL *carry;
  int *pinloc, *pinsum, *pinflag;
  // Agenten-SIR
  REAL *SIR[3];
  int *SIRmode; // SIRmode=0: S
                // SIRmode=1: E 
                // SIRmode=2: I
                // SIRmode=3: R
  int MoveMode;
} VIDEO;


typedef struct {
  REAL PI;      // init: percent infected (not yet included)
  REAL CD;      // critical distance
  REAL IR;      // infection rate
  REAL RT;      // ressistence time (Verweildauer)
  REAL PR;      // percent of removed (not yet included)
  int SEIRMonitoring[4];
} SIRagent;


