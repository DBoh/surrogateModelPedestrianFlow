/*!
 ****************************************************************************
 *                                                                          
 *  \file    std.h                                                          
 *                                                                          
 *  \brief   pFlowSim Preprocessor Flags                                    
 *                                                                          
 *  \details Building pFlowSim with/without Plots and Messages requires 
 *           setting the preprocessor flags PLOT und MSG
 *   
 * PLOT=0/1: on/off for plots (octave)  
 * MSG=0/1:  on/off messages at stdout 
 * TP=0/1:   on/off computing agents (tracking-points) based on rho-vel-data 
 *  
 * EOC=1: Test Helmholtz  
 * EOC=2: Test Kontinuitaet 
 * Test Vel und Clement
 * EOC=3: Test gekoppeltes System
 *  
 * WEB=0/1: build up WebUI 
 *  
 *  
 **************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define VRTrack   0
#define REALTYPE  1
#define EOC  0

#define TP   1
#define SIRA 1

#define GUI 0  
#define WEB 0
#define VR  0

#if WEB || VR
#define MSG       0
#define PLOT      0
#else
#define MSG       1
#define PLOT      2
#endif

#if MSG || WEB
#define WMSG 1
#endif

#if REALTYPE==1
  typedef double REAL;
  #define readbufc( ... )  sscanf(linebuf , "%s %lf "  , bla, __VA_ARGS__ )
  #define freadbuff( ... ) fscanf(fp      , "%lf %lf " ,      __VA_ARGS__ )
  #define freadbufc( ... ) fscanf(fp      , "%s %lf "  , bla, __VA_ARGS__ )
#else
  typedef float REAL;
  #define readbufc( ... ) sscanf(linebuf,"%s %f ",bla, __VA_ARGS__ )
  #define freadbuff( ... ) fscanf(fp,"%f %f ", __VA_ARGS__ )
  #define freadbufc( ... ) fscanf(fp,"%s %f ",bla, __VA_ARGS__ )
#endif
