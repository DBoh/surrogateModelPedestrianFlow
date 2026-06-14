clear
close

Flags = readtable("../Octave/MatlabFlags.dat");

fc_orange = '#F1851F';
fc_gruen  = '#9CC41B';
fc_dblau  = '#041E2A';
fc_drot   = '#BE1F21';
fc_drblau = '#496572';

AllColors(1,:) = fc_orange;
AllColors(2,:) = fc_gruen;
AllColors(3,:) = fc_dblau;
AllColors(4,:) = fc_drot;
AllColors(5,:) = fc_drblau;

MONI = readtable("../00Output/Monitoring.dat");
M  = length(MONI.t);
FM = fieldnames(MONI);
N  = length(FM);

EntreeC = cumsum(MONI.entree);

numExit = 0;
name = sprintf('exit_%d',numExit);
for l=1:N
    if length(FM{l})==length(name)
        if sum(FM{l}==name)==length(name)
            fprintf('name=%s\n',name);
            if numExit==0; ExitStart=l; end
            ExitEnd = l;
            numExit = numExit+1; 
            name = sprintf('exit_%d',numExit);
            switch numExit
                case 1
                    Exits(:,numExit) = MONI.exit_0;
                    AllExits = Exits(:,1);
                case 2
                    Exits(:,numExit) = MONI.exit_1;
                    AllExits = AllExits + Exits(:,2);
                case 3
                    Exits(:,numExit) = MONI.exit_2;
                    AllExits = AllExits + Exits(:,3);
                case 4
                    Exits(:,numExit) = MONI.exit_3;
                    AllExits = AllExits + Exits(:,4);
                case 5
                    Exits(:,numExit) = MONI.exit_4;
                    AllExits = AllExits + Exits(:,5);
                otherwise
                    fprintf('Anzahl Exits hochsetzen!!!');
            end
        end
    end
end


AllExitsC = cumsum(AllExits);
ExitsC = cumsum(Exits);

%%=================================================================
% measurment-stations

numMS = 0;
name = sprintf('MS_%d',numMS);
for l=1:N
    if length(FM{l})==length(name)
        if sum(FM{l}==name)==length(name)
            fprintf('name=%s\n',name);
            if numMS==0; MSStart=l; end
            MSEnd = l;
            numMS = numMS+1; 
            name = sprintf('MS_%d',numMS);
            switch numMS
                case 1
                    MS(:,numMS) = MONI.MS_0;
                case 2
                    MS(:,numMS) = MONI.MS_1;
                case 3
                    MS(:,numMS) = MONI.MS_2;
                case 4
                    MS(:,numMS) = MONI.MS_3;
                case 5
                    MS(:,numMS) = MONI.MS_4;
                otherwise
                    fprintf('Anzahl MS hochsetzen!!!');
            end
        end
    end
end

% h = 0.032766
% max(MONI.exit_0)-max(MONI.MS_0) = 0.1927 - 0.1572 = 0.0355
% max(MONI.exit_1)-max(MONI.MS_1) = 0.1682 - 0.1592 = 0.0090
% h = 0.019678
% max(MONI.exit_0)-max(MONI.MS_0) = 0.1976 - 0.1700 = 0.0276
% max(MONI.exit_1)-max(MONI.MS_1) = 0.1882 - 0.1739 = 0.0143

%%=======================================================================
% people inside left entered
h = figure(1);
set(h, 'Position', [0 0 1920 1080])

subplot(2,3,1)
plot(10.*MONI.t,MONI.mass,'color',fc_dblau);
title('people in the house')
grid on
hold on
plot(10.*MONI.t,AllExitsC,'color',fc_gruen);
if sum(MONI.entree)>0
    plot(10.*MONI.t,cumsum(MONI.entree),'color',fc_orange);
    legend('people inside','people left (cum)','peple entered (cum)');
else
    legend('people inside','people left (cum)');
end
xlabel('time (sec)');
ylabel('number of people')
% SEIR
subplot(2,3,3)
plot(MONI.t,MONI.e,'color',fc_gruen);
title('those who got it')
grid on
hold on
legend(sprintf('infected people (%.2f pC infectious)',MONI.i(1)*100/MONI.mass(1)));
xlabel('time (sec)');
ylabel('number of people')
%
subplot(2,3,4)
plot(MONI.t,Exits(:,1),'color',AllColors(1,:));
bla(1,:) = sprintf('door %d',0);
title('the coming and going')
grid on
hold on
for i=2:numExit
    plot(MONI.t,Exits(:,i),'color',AllColors(i,:));
    bla(i,:) = sprintf('door %d',i-1);
end    
legend(bla);
if sum(MONI.entree)>0
    plot(MONI.t,MONI.entree,'color',fc_orange);
end
xlabel('time (sec)');
ylabel('number of people')

clear bla
subplot(2,3,6)
if numMS
    plot(MONI.t,10.*MS(:,1),'color',AllColors(1,:));
    %bla(1,:) = sprintf('people gone through MS %d',0);
    title('bottleneck')
    grid on
    hold on
    for i=2:numMS
        plot(MONI.t,10.*MS(:,i),'color',AllColors(i,:));
        %bla(i,:) = sprintf('people gone through MS %d',i-1);
    end
    legend(bla);
    xlabel('time (sec)');
    ylabel('number of people')
end
clear bla
subplot(2,3,5)
plot(MONI.t,ExitsC(:,1),'color',AllColors(1,:));
bla(1,:) = sprintf('door %d',0);
title('the coming and going (cumulative)')
grid on
hold on
for i=2:numExit
    plot(MONI.t,ExitsC(:,i),'color',AllColors(i,:));
    bla(i,:) = sprintf('door %d',i-1);
end    
legend(bla);
xlabel('time (sec)');
ylabel('number of people')

subplot(2,3,2)
[ax, h1, h2] = plotyy(MONI.t,MONI.rhomin,MONI.t,MONI.rhomax);
title('crowded room')
set(h1,'color',fc_orange);
ax(1).YColor = fc_orange;
set(h2,'color',fc_drot);
ax(2).YColor = fc_drot;

grid on
legend('Rhomin','Rhomax');
xlabel('time (sec)')
ylabel('rhomin, rhomax')


print(h,'-dpng','Monitoring.png')