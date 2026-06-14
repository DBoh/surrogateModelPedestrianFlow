clear all
close all

addpath ../Octave/

h=figure(1)

Phi_0000000
hold on
Dist_0000000


xlabel('x')
ylabel('y')

print(h,'-dpng','PlotPhi.png')