void Exchange(REAL *x, REAL *y);
void ErrorMessage(char *, char *);
void Message(char *, char ,char *);
void FieldCpInt(int *, int , int *, int , int );
void FieldCpREAL(REAL *, REAL , REAL *, REAL ,int );
void FieldCp(REAL *, REAL , REAL *, REAL ,int );
void FieldMl(REAL *, REAL , REAL *, REAL *, REAL , int );
void field_cp(REAL *, REAL , REAL *, REAL , REAL *, REAL , int );
void add_numer_to_string(char *, int , int );
REAL norm(REAL *, int , REAL );
REAL norm2(REAL *, int );
void print_field(REAL *, int , int , char *);
void PrintFieldMinMax(REAL , REAL *, int , REAL *, REAL *, char *, char *);
int FieldMin_int(int * , int);
int FieldMax_int(int * , int);
REAL FieldMin(REAL *, int );
REAL FieldMax(REAL *, int );



void DistField(REAL *d, REAL ax, REAL ay, REAL bx, REAL by);
void OrthogonalProjection(REAL *xp, REAL *yp, REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by);
int SegToDomainBoundMagnet(REAL *s1x, REAL *s1y, REAL *s2x, REAL *s2y);
REAL dist(REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by);
REAL DistSeg(REAL x, REAL y, REAL ax, REAL ay, REAL bx, REAL by);
void FuncVec(REAL t, REAL *v, REAL (*f)(REAL , REAL , REAL ));
void DomainPolygonsAndOrientations();
void PolyHoles();

