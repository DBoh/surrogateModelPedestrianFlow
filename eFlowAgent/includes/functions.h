int signum(REAL );

void TriRandXY(int k, REAL xt[2]);
void SetDecomposition(int N, int M, int *Nj);
REAL PolyArea(REAL *P[2], int N);
REAL RandVal();
REAL DistPointLineSegment(REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL By);
int PointElementSegment(REAL , REAL , REAL , REAL , REAL , REAL );
int IntersectionSegment(REAL ,REAL ,REAL ,REAL ,REAL ,REAL ,REAL ,REAL ,REAL *,REAL *,REAL *,REAL *);
int SegBoundarySchnitt(REAL ax, REAL ay, REAL bx, REAL by, int *kk, int *ll);
int SegSchnitt(REAL ,REAL ,REAL ,REAL ,REAL *,REAL ,REAL ,REAL ,REAL ,REAL *);
REAL schnitt(REAL ,REAL ,REAL ,REAL ,REAL ,REAL ,REAL ,REAL );

REAL QuadSegment(int , REAL , REAL , REAL , REAL , REAL *, REAL *[2], int );
int SegPolySchnitt(REAL ax, REAL ay, REAL bx, REAL by, POLY P);

void LinePlot(REAL , REAL , REAL , REAL , REAL *, char *, int );


void DiffQuot(REAL *, REAL *, REAL *, int );

void WriteRhoVel(REAL *, REAL *[2],char *,int );
void ReadRhoVel(REAL *, REAL *[2],char *,int );

void WriteTrackPoints(REAL *, char *, int );
void ReadTrackPoints(REAL *, char *, int );
void WriteVRTrack(REAL *rho, REAL *v[2],char *fname,REAL time, char *type);
void WriteLog(char *,char *);

REAL min_REAL(REAL ,REAL );
REAL max_REAL(REAL ,REAL );
int min(int ,int );
int max(int ,int );

void ExitDistFunc_Vec(REAL *);
REAL ExitDistFunc(REAL , REAL , REAL );

REAL VecVecIntegral(REAL *, REAL *, REAL *);

int CumSumInt(int *v, int N);
REAL CumSum(REAL *v, int N);
REAL CumSumAbs(REAL *v, int N);

int PunktInPolygon(REAL Qx,REAL Qy,REAL *P[2],int N);
int PointInPoly(REAL Qx, REAL Qy, POLY P);


REAL ConvMass1dElem(REAL t, REAL s);
