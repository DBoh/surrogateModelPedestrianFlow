void standardvalues();                                                // values.c
void dimensionalisation();

REAL cspline_loc(int , REAL [4][4], REAL , REAL );                    // spline_loc.c

void uexact(REAL *);                                                  // data.c
REAL uex(int );                                                       // data.c
REAL u_ex(REAL , REAL , int );                                        // data.c
REAL du_ex(int , REAL , REAL , int );                                 // data.c
void empty_vec(REAL *, int , REAL );                                  // data.c

void ErrorVecFunc(REAL , REAL *[2], REAL (*)(REAL , REAL , REAL ), REAL (*)(REAL , REAL , REAL ), REAL (*)(REAL , REAL , REAL , int ), REAL (*)(REAL , REAL , REAL , int ), char *);
void ErrorFunc(REAL , REAL *, REAL (*)(REAL , REAL , REAL ), REAL (*)(REAL , REAL , REAL , int ), char *);
REAL L2ErrorFunc(REAL , REAL *, REAL (*)(REAL , REAL , REAL ));
REAL H1ErrorFunc(REAL , REAL *, REAL (*)(REAL , REAL , REAL , int ));

