void PrintSparceMatrix(REAL *, int , char *);                         // prints.c

void u_scale(REAL *, REAL *, int );                                   // graphics.c
int schnitt_ebene(REAL [3],REAL [3],REAL ,REAL [3]);                  // graphics.c
void u_minmax_scale(REAL *, REAL *, REAL , REAL , int );              // graphics.c


void octavesubdomplot();
void octavegridplot();
void octaveplot(REAL *, char *, REAL , REAL , int );                  // octaveplot.o
void octaveplotvec(REAL *[2], char *,int );                           // octaveplot.o
void octaveplotdomain();
int OctavePlotAll(REAL *, REAL *[2], REAL *, int , int );



void TrackPointInit();
void TrackPoint(REAL *, REAL *[2], int );
void AddEntreeTracks(int );
int tracking(REAL *, REAL *, REAL *[2], REAL *, REAL , int *);

void ProfileLine(REAL , REAL , REAL , REAL , REAL *, char *);

void PED_bound();

