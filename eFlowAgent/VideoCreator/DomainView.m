clear all
close all

Flags = readtable("../Octave/MatlabFlags.dat");

AttFlag = 0; %Flags.AttFlag;
SubdomFDFlag = Flags.SDFDFlag;
SubdomRhoFlag = 1;%Flags.SDRhoFlag;
TrackInitFlag = 0;
MSFlag = Flags.NumMS;
dOmegaFlag = 1;
Doorflag = Flags.NumExit;
numfig=3;

% Colors
Gelb = [243 224 17]/255;
Gruen = [155 185 25]/255;
Rot = [190 31 33]/255; %[1 0 0];
Blau = [4 30 42]/255;


Tx = load("../Octave/Gridx.dat")';
Ty = load("../Octave/Gridy.dat")';
dOx = load("../00Output/dOmegaPx.dat")';
dOy = load("../00Output/dOmegaPy.dat")';

if AttFlag
    SAx = load("../Octave/SubdomAttx.dat")';
    SAy = load("../Octave/SubdomAtty.dat")';
    SAc = load("../Octave/SubdomAttc.dat")';

    Attractor = load("../Octave/Attractors.dat");
    [MA , NA] = size(Attractor);
end

if SubdomFDFlag
    SDCVx = load("../Octave/SubdomFDx.dat")';
    SDCVy = load("../Octave/SubdomFDy.dat")';
    SDCVc = load("../Octave/SubdomFDc.dat")';
end

if SubdomRhoFlag
    RhoInix = load("../Octave/SubdomRhoinix.dat")';
    RhoIniy = load("../Octave/SubdomRhoiniy.dat")';
    RhoInic = load("../Octave/SubdomRhoinic.dat")';
end

if TrackInitFlag
    TI = load("../Octave/TrackPoints_i_x_y_rho_0000000.dat");
end
    
MEx = 0; NEx = 0; MEn = 0; NEn = 0;
if Doorflag
    Exit = load("../Octave/Exits.dat");
    [MEx , NEx] = size(Exit);
    Entree = load("../Octave/Entrees.dat");
    [MEn , NEn] = size(Entree);
end

MMSx = 0; NMSx = 0;
if MSFlag
    MSx = load("../Octave/MS.dat");
    [MMSx , NMSx] = size(MSx);
end

h=figure(numfig)
patch(Tx,Ty,[0.8,0.8,0.8]);
daspect([1 1 1])
hold on
patch(dOx,dOy,[0.8,0.,0.]);


for i=1:MEx
    plot([Exit(i,1),Exit(i,3)],[Exit(i,2),Exit(i,4)],'color',[0 0.5 0],'LineWidth',5);
end
for i=1:MEn
    plot([Entree(i,1),Entree(i,3)],[Entree(i,2),Entree(i,4)],'color',[0.5 0 0],'LineWidth',5);
end
for i=1:MMSx
    plot([MSx(i,1),MSx(i,3)],[MSx(i,2),MSx(i,4)],'color',[0.7 0.7 0],'LineWidth',5);
end

if AttFlag
    patch(SAx,SAy,SAc); 

    for i=1:MA
        plot([Attractor(i,1),Attractor(i,3)],[Attractor(i,2),Attractor(i,4)],'color',[0.5 0.5 0],'LineWidth',5);
    end
end

if SubdomFDFlag
    patch(SDCVx,SDCVy,SDCVc);
end
if SubdomRhoFlag
    patch(RhoInix,RhoIniy,RhoInic);
end
%colorbar

if TrackInitFlag
    plot(TI(:,2),TI(:,3),'o','MarkerEdgeColor','r','MarkerFaceColor',[1 0. 0.],'MarkerSize',10);
end
print(h,'-dpng','Domain.png')