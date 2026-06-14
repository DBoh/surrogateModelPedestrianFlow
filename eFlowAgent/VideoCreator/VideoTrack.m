close all
clear all

% Palette [58 38 38] pink, [92 92 73] gelb
% [30 30 4] dunkelgruen, [70 70 410] hellgruen

global Path GridView ExitCol EntreeCol
Path="../";  % Pfad wo sich das Octave-Verzeichnis befindet

fstart = 0;
fend = 20;
fps = 10; % should be 1/PED.delta

% Views 
GridView = 0; % nur umstellen, wenn du weisst was du tust ;-)
VelView = 1;
TPView = 0;
RhoView = 1;

Videoname = "eFlowAgent.avi";
CreateVideo = 1;
TPnotEmpty = 1;

% Falls das Gitter angezeigt werden soll ist es ratsam einmalig ein
% Hintergrundbild zu erzeugen. Ist sonst zu rechenaufwendig
BG = 0;
% Setting braucht man nur bei GridView
Setting="EOCLittleDoors"; % Labyrinth, Nik, NikO, EOC, Seminar

HDscal = 1;
TPcol  = [70  70 10]/100;
TPcolb = [30 30 4]/100;
Velcol = [0.5,0.5,0.5];

ExitCol = [50 70 10]/100;
EntreeCol = [100 65 65]/100;
% SIR-colors
ColS = [243 224 17]/255;
ColE = [155 185 25]/255;
ColI = [190 31 33]/255; %[1 0 0];
ColR = [4 30 42]/255;

RhoMinMax = [0,1];
[Ix,Iy,bgname,RhoMinmax] = GetParam(Setting);

%%
FS = 1920*HDscal;
MS = 15*HDscal;

figure('Name','TrackingPoints','Position',[0,0,FS,FS*1080/1920]);
if CreateVideo
    v = VideoWriter(Videoname);
    v.FrameRate = fps;
    v.Quality = 85;
    open(v);
end

if BG==1
    % create a background-grid-picture
    Exit   = load(sprintf('%s/Octave/Exits.dat',Path));
    Entree = load(sprintf('%s/Octave/Entrees.dat',Path));
    Domain = load(sprintf('%s/Octave/SubdomRho.dat',Path));
    
    PlotPatches(Domain,Exit,Entree,MS*0.0008);
    daspect([1 1 1]);
    hold on
    
    frame = getframe(gcf);
    [F, Map] = frame2im(frame);
    imwrite(F,"pFlow_bg.png");
    
else
    
    if GridView
        PlotBackground(bgname,Ix,Iy);
    else
        [Ix, Iy]=GridLimits();
        xlim(Ix);
        ylim(Iy);
        
        Bound  = load(sprintf('%s/Octave/Domain.dat',Path));
        Exit   = load(sprintf('%s/Octave/Exits.dat',Path)); 
        Entree = load(sprintf('%s/Octave/Entrees.dat',Path));
        
        
     end
    da=(Iy(2)-Iy(1))/(Ix(2)-Ix(1));
    daspect([1 1 1]);
    hold on
    
    if ~RhoView
       % Tx = load("../Octave/Gridx.dat")';
        %Ty = load("../Octave/Gridy.dat")';
        %patch(Tx,Ty,[0.8 0.8 0.8]);
        %hold on
    end

    if RhoView
        bound = load(sprintf('%s/Octave/dOmega.dat',Path));
        [MB,N]= size(bound);
        
        x1=load(sprintf('%s/Octave/Rho_x_0000000.dat',Path));
        x2=load(sprintf('%s/Octave/Rho_y_0000000.dat',Path));
        [xx,yy]=meshgrid(x1,mirror(20.*x2,20.*Iy));
    end
    
    for i=fstart:fend
        time = i/fps;
        titletext = sprintf('time = %.2f',time);
        
        
        if TPnotEmpty && (i>fstart && TPView); delete(a); end
        if i>fstart && VelView; delete(b); end
        if i>fstart && RhoView; delete(c); end
        
        if RhoView
            name=sprintf('%s/Octave/Rho_z_%07d.dat',Path,i);
            zz=load(name);
            c=surf(xx,yy,6.*zz);
            %colormap turbo 
            colormap jet(10)
            shading interp
        end
        
        if ~GridView && i==fstart
            PlotBound(Bound,Exit,Entree);
        end
        
        if VelView
            [xx,yy,uu,vv]=GetVelData(i,Iy);
            zz = ones(size(xx))*2;
            ww = ones(size(xx))*2;
            %h=figure(1)
            b=quiver3(xx,yy,zz,uu,vv,ww,'color',Velcol);
            %print(h,'-dpng','PlotVelocity.eps')
        end
        
        if TPView
            name=sprintf('%s/Octave/TrackPoints_i_x_y_rho_%07d.dat',Path,i);
            Q=load(name);
            [M N]=size(Q);
            TPnotEmpty = 0;
            if(M)
                ymirror = mirror(Q(:,3),Iy);

                IndS = find(Q(:,5)==0);
                IndE = find(Q(:,5)==1);
                IndI = find(Q(:,5)==2);
                IndR = find(Q(:,5)==3);

                if length(IndS); a(1) = plot3(Q(IndS,2),ymirror(IndS),3*ones(length(IndS),1),'o','Color',ColS*0.3,'MarkerFaceColor',ColS,'MarkerSize',MS); end
                if length(IndE); a(2) = plot3(Q(IndE,2),ymirror(IndE),3*ones(length(IndE),1),'o','Color',ColE*0.3,'MarkerFaceColor',ColE,'MarkerSize',MS); end
                if length(IndI); a(3) = plot3(Q(IndI,2),ymirror(IndI),3*ones(length(IndI),1),'o','Color',ColI*0.3,'MarkerFaceColor',ColI,'MarkerSize',MS); end
                if length(IndR); a(4) = plot3(Q(IndR,2),ymirror(IndR),3*ones(length(IndR),1),'o','Color',ColR*0.3,'MarkerFaceColor',ColR,'MarkerSize',MS); end
                %            a = plot3(Q(:,2),ymirror,3*ones(M,1),'o','Color',TPcolb,'MarkerFaceColor',TPcol,'MarkerSize',MS);

                TPnotEmpty = 1;
            end
        end
            title(sprintf('time = %.2f',time))

            frame = getframe(gcf);
        if CreateVideo
            title(titletext);
            frame = getframe(gcf);
            writeVideo(v,frame);
        end
    end
    
    %    fprintf('Rhomax = %f\n',RhoMa);
    if CreateVideo; close(v); end
    
    
end



%%
function [Ix, Iy]=GridLimits()

global Path

x1   =load(sprintf('%s/Octave/Vel_x_0000000.dat',Path));
x2tmp=load(sprintf('%s/Octave/Vel_y_0000000.dat',Path));

Ix = [min(x1),max(x1)];
Iy = [min(x2tmp),max(x2tmp)];
end


function [xx,yy,zz1,zz2]=GetVelData(loop,Iy)
global Path GridView

x1   =load(sprintf('%s/Octave/Vel_x_0000000.dat',Path));
x2tmp=load(sprintf('%s/Octave/Vel_y_0000000.dat',Path));
x2 = mirror(x2tmp,Iy);

zz=load(sprintf('%s/Octave/Vel_z_0000000.dat',Path));
N=length(x1);
M=length(x2);
[xx,yy]=meshgrid(x1,x2);

name=sprintf('%s/Octave/Vel_z_%07d.dat',Path,loop);
zz=load(name);

for i=1:M
    for j=1:N
        zz1(i,j)=zz(i,2*j-1);
        zz2(i,j)=zz(i,2*j);
    end
end

if GridView
    zz2 = -zz2;
end

end

function xm=mirror(x,I)
global GridView

if GridView
    xm = ((x-I(1))*I(1)+(I(2)-x)*I(2))/(I(2)-I(1));
else
    xm = x;
end

end

function PlotBackground(bgname,Ix,Iy)

hold off
xlim(Ix); ylim(Iy);
I = imread(bgname);
h = image(xlim,ylim,I);
uistack(h,'bottom')
daspect([1 1 1]);
hold on

end

function PlotPatches(Q,Exit,Entree,eps)


[M N] = size(Q);

for i=1:M
    patch([Q(i,1),Q(i,2),Q(i,3)],[Q(i,4),Q(i,5),Q(i,6)],[Q(i,7),Q(i,8),Q(i,9)]);
end

end

function PlotExitEntree(Exit,Entree)
global ExitCol EntreeCol

[NumEx ,N] = size(Exit);
for l=1:NumEx
    Axmin = min([Exit(l,1),Exit(l,3)])-eps;
    Axmax = max([Exit(l,1),Exit(l,3)])+eps;
    Aymin = min([Exit(l,2),Exit(l,4)])-eps;
    Aymax = max([Exit(l,2),Exit(l,4)])+eps;
    
    X1 = Exit(:,1)'; X2 = Exit(:,3)';
    Y1 = Exit(:,2)'; Y2 = Exit(:,4)';
    [M N]=size([X1; X2]);
    plot3([X1; X2], [Y1; Y2], 2*ones(M,N),'LineWidth',4,'color',ExitCol);

end

[NumEn ,N] = size(Entree);
for l=1:NumEn
    Axmin = min([Entree(l,1),Entree(l,3)])-eps;
    Axmax = max([Entree(l,1),Entree(l,3)])+eps;
    Aymin = min([Entree(l,2),Entree(l,4)])-eps;
    Aymax = max([Entree(l,2),Entree(l,4)])+eps;
    X1 = Entree(:,1)'; X2 = Entree(:,3)';
    Y1 = Entree(:,2)'; Y2 = Entree(:,4)';
    [M N]=size([X1; X2]);
    plot3([X1; X2], [Y1; Y2], 2*ones(M,N),'LineWidth',4,'color',EntreeCol);
end

end



function [Ix,Iy,bgname,RhoMinMax] = GetParam(Type)

switch Type
    case "Wall"
        bgname = sprintf('WallA_bg.png');
        
        Ix = [0.,1.2];
        Iy = [0.,1.];
        
    case "EOC"
        bgname = sprintf('EOC_bg.png');
        
        Ix = [0,1.2];
        Iy = [-0.1,0.6];
        RhoMinMax = [0,0.341310];
    case "EOCLittleDoors"
        bgname = sprintf('EOCLittleDoors_bg.png');
        
        Ix = [-0.2,1.2];
        Iy = [0,0.5];
        RhoMinMax = [0,0.341310];
    case "Nik"
        %bgname = sprintf('ExportNikFine_bg.png');
        bgname = sprintf('ExportNik_bg.png');
        %bgname = sprintf('EOC_bg.png');
        
        Ix = [0,1.2];
        Iy = [0,0.6];
    case "NikO"
        %        bgname = sprintf('ExportNikO.png');
        bgname = sprintf('ExportNik2Medium_bg.png');
        RhoMinMax = [0.000000, 0.159071];
        
        Ix = [0,1.0];
        Iy = [0,0.9];
    case "Seminar"
        bgname = sprintf('Seminarraum_bg.png');
        
        Ix = [0,1.2];
        Iy = [0,2];
    case "Labyrinth"
        bgname = sprintf('Labyrinth_bg.png');
        
        Ix = [0,1.];
        Iy = [0,1.2];
    otherwise
        display('unknown Type');
end
end


function PlotBound(P,Exit,Entree)

X1=P(:,1)';
Y1=P(:,2)';
X2=P(:,3)';
Y2=P(:,4)';

[M N]=size([X1; X2]);

plot3([X1; X2], [Y1; Y2], 2*ones(M,N),'LineWidth',2,'color',[1 1 1]);

PlotExitEntree(Exit,Entree);

end
