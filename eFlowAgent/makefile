PLOTLIB = -L/usr/lib -lX11
VPATH=./:./Base/:./LGS/:./Grid/:\
	./Graphics/:./LGS/cblas/:./Functions: \
	./Tools:./PED:./FD:./Tests:./Agents: 

includes=-Iincludes

basefiles=tools.o values.o quadratur.o

picfiles=graphics.o 

agentfiles=sir.o

graphicfiles=octaveplot.o tracking.o

gridfiles=grid_IO.o triang.o grid_ofiles.o assign.o \
	subdom.o tools_functions.o domain.o

lgsfiles=matrizen.o matrix_functions.o vector_functions.o \
	rhs.o lgs_ofiles.o prints.o bound_integrals.o \
	velocity.o QuasiEikonal.o QuasiEikonal2.o

trianglefiles=triangle.o meshgen.o

femfiles=$(gridfiles) $(graphicfiles) $(basefiles) $(lgsfiles) \
	spline_loc.o error.o $(trianglefiles) $(agentfiles)\
	cg.o data.o solver.o write.o func.o equations.o \
	exacts.o conv.o fastsweeping.o

fdfiles=ped_fundamental.o ped_fd.o

testfiles=eocfunc.o MatrixTest.o

pedestrianfiles=$(femfiles) $(fdfiles) ped_boundary.o \
	ped_functions.o ped_init.o plots.o json_param.o \
	ped_ofiles.o ped_param.o ped_monitoring.o

videofiles=$(pedestrianfiles) 

F77  = g77
gnuC = gcc $(includes)

ifeq ($(WEB), 1)
gnuC = $(EMSCRIPTEN_CC) $(includes)
else
gnuC = gcc $(includes)
endif
ifeq ($C,1)
gnuC += -g -Wall -pedantic
else ifeq ($C,3)
gnuC += -Wall -pg
else ifeq ($C,4)
gnuC += -Warray-bounds -pg
else ifeq ($C,2)
gnuC += -g -O0 
else
gnuC += 
endif

ifeq ($(WEB), 1)
pFlowSIM:pFlowSIM.o $(pedestrianfiles) 
	$(gnuC) pFlowSIM.o $(pedestrianfiles) -r -o pFlowSIM.bc -lm
else
pFlowSIM:pFlowSIM.o $(pedestrianfiles) 
	$(gnuC) pFlowSIM.o $(pedestrianfiles)  -o pFlowSIM -lm 

json2eflow:json2eflow.o 
	$(gnuC) json2eflow.o -o json2eflow -lm 

pFlowBench:pFlowBench.o $(pedestrianfiles) 
	$(gnuC) pFlowBench.o $(pedestrianfiles)  -o pFlowBench -lm 

pFlowVeri:pFlowVeri.o $(pedestrianfiles) 
	$(gnuC) pFlowVeri.o $(pedestrianfiles)  -o pFlowVeri -lm 

geometrie:geometrie.o $(pedestrianfiles) 
	$(gnuC) geometrie.o $(pedestrianfiles)  -o geometrie -lm 

pFlowTEST:pFlowTEST.o $(pedestrianfiles) $(testfiles)
	$(gnuC) pFlowTEST.o $(pedestrianfiles) $(testfiles) -o pFlowTEST -lm 
endif

.f.o:   
	$(F77)  -c $<
.c.o:
	$(gnuC) -c $<

.PHONY:	clean new 

clean:
	rm -f *.o *~ pFlowSIM pFlowTEST pFlowVeri *.log

cleansrc:
	rm -f *.o *~ *.log

cleantilde:
	rm -f Base/*~ LGS/*~ Grid/*~ includes/*~ \
	Graphics/*~ PPM_tools/*~ LGS/cblas/*~ 00Input/*~ 00Output/*~ \
	Tools/*~ PED/*~ Functions/*~ *~


new:	clean pFlowSIM 



#valgrind: >make clean C=1 pFlow
#> valgrind --leak-check=full  --show-leak-kinds=all --log-file=val.out -v ./pFlowSIM 

#gprof: > make C=3
#> ./pFlowSIM
#> gprof pFlowSIM gmon.out > analysis.txt

