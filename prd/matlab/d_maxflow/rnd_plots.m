commandwindow;
solvers={
	'BK',
	'GT0',
	'GT05',
	'HPR',
	'S-ARD',
	'S-PRD'
	};
solvers_legend={
	'BK',
	'HIPR0',
	'HIPR0.5',
	'HPR',
	'S-ARD',
	'S-PRD'
	};
%pth = 'G:\personal\shekhovt\dev\data\d_maxflow\rnd2/';
%pth = '../../data/d_maxflow/rnd/';
%pth = '../../data/d_maxflow/rnd2/';
%pth = '../../data/d_maxflow/rnd4/';Str = 200;
%pth = '../../data/d_maxflow/rnd_s45/';Str = 45;
%pth = '../../data/d_maxflow/rnd_s450/';Str = 600;
pth = '../../data/d_maxflow/rnd_s600/';Str = 600;
%
if(exist([pth 'lines.mat'],'file'))
	load([pth 'lines']);
else
	lines = get_reports(pth,solvers);
	save([pth 'lines'],'lines');
end
%
for i=1:numel(lines)
	if(isfield(lines{i},'name'))
		p1 = strfind(lines{i}.name,'_conn');
		p2 = strfind(lines{i}.name,'_inst');
		conn = str2double(lines{i}.name(p1+length('_conn'):p2-1));
		lines{i}.conn = conn;
		p3 = strfind(lines{i}.name,'_str');
		if(isempty(p3))
			str = 100;
		else
			str = sscanf(lines{i}.name(p3+length('_str'):end),'%f',1);
		end
		lines{i}.str = str;
	end
end

solve = get_cell_field(lines,'solve');
msg = get_cell_field(lines,'msg');
augment = get_cell_field(lines,'augment');
relabel = get_cell_field(lines,'relabel');
gap = get_cell_field(lines,'gap');
vert = get_cell_field(lines,'vert');
vgroups = unique(vert(:));
conn = get_cell_field(lines,'conn');
sweeps = get_cell_field(lines,'sweeps');
work = msg+augment+relabel+gap;
work(:,1:4) = solve(:,1:4);
split = get_cell_field(lines,'Regions');
regions = split.^2;
str = get_cell_field(lines,'str');
sgroup = unique(str(:));

style = {'-g','--k',':k','--m','-ob','-sr'};
%% strength
sgroup(sgroup==0) = [];
c = 4;
x = [];
v = [];
v_cl = [];
v_cu = [];
w = [];
w_cl = [];
w_cu = [];
for(i=1:length(sgroup))
	m = (vert==1000002) & (conn==c) & (regions==4) & (str==sgroup(i));
	x(i) = floor(sgroup(i)/c);
	for j=1:length(solvers_legend)
		[y ci] = sim_eci(sweeps(m(:,5),j),0.3);
		v(i,j) = y;
		v_cl(i,j) = ci(1)-y;
		v_cu(i,j) = ci(2)-y;
		%
		[y ci] = sim_eci(work(m(:,5),j),0.3);
		w(i,j) = y;
		w_cl(i,j) = ci(1)-y;
		w_cu(i,j) = ci(2)-y;
	end
end
figure(1); clf; subplot(2,1,1);hold on;
for s=5:length(solvers_legend)
	errorbar(x',v(:,s),v_cl(:,s),v_cu(:,s),style{s});
end
h = legend(solvers_legend{5:end});
set(h,'Location','Best');
set(h, 'interpreter', 'latex');
if length(x)>1
	mxlim([min(x) max(x)]);
end
ylim([0 inf]);
mxlim([min(x) max(x)]);
set(gca,'XTick',[50    100  150   200   250   300   350   400   450   500]);
xlabel('strength');
ylabel('number of sweeps');
%figure(2); clf;
subplot(2,1,2);hold on;
for s=1:length(solvers_legend)
	errorbar(x',w(:,s),w_cl(:,s),w_cu(:,s),style{s});
end
h = legend(solvers_legend);
set(h,'Location','NorthEast');
set(h, 'interpreter', 'latex');
mxlim([min(x) max(x)]);
%set(gca,'XTick',x);
set(gca,'XTick',[50    100  150   200   250   300   350   400   450   500]);
xlabel('strength');
ylabel('CPU, sec.');

%o = findall(gcf,'Interpreter','tex');
%set(o, 'interpreter', 'latex');

pdffig([pth '/plots/strength.pdf']);
%return
%% size
vgroups(vgroups==0) = [];
vgroups(vgroups<200*200) = [];
c = 4;
x = [];
v = [];
v_cl = [];
v_cu = [];
w = [];
w_cl = [];
w_cu = [];
for(i=1:length(vgroups))
	m = (vert==vgroups(i)) & (conn==c) & (regions==4) & (str==Str);
	x(i) = sqrt(vgroups(i)-2);
	for j=1:length(solvers_legend)
		[y ci] = sim_eci(sweeps(m(:,5),j),0.3);
		v(i,j) = y;
		v_cl(i,j) = ci(1)-y;
		v_cu(i,j) = ci(2)-y;
		%
		[y ci] = sim_eci(work(m(:,5),j),0.3);
		w(i,j) = y;
		w_cl(i,j) = ci(1)-y;
		w_cu(i,j) = ci(2)-y;
	end
end
figure(1); clf; subplot(2,1,1); hold on;
for s=5:length(solvers_legend)
	errorbar(x',v(:,s),v_cl(:,s),v_cu(:,s),style{s});
end
h = legend(solvers_legend{5:end});
set(h,'Location','Best');
set(h, 'interpreter', 'latex');
ylim([0 350]);
mxlim([min(x) max(x)]);
set(gca,'XTick',x);
xlabel('vertices^{1/2}');
ylabel('number of sweeps');
set(gca,'YTick',[8 50 100 150 200 250 300 350]);
%figure(2); clf;hold on; 
subplot(2,1,2); hold on;
for s=1:length(solvers_legend)
	errorbar(x',w(:,s),w_cl(:,s),w_cu(:,s),style{s});
end
h = legend(solvers_legend);
set(h, 'interpreter', 'latex');
set(h,'Location','Best');
%mxlim([min(x) max(x)]);
ylim([0 inf]);
mxlim([min(x) max(x)]);
set(gca,'XTick',x);
xlabel('vertices^{1/2}');
ylabel('CPU, sec.');
%
%o = findall(gcf,'Interpreter','tex');
%
pdffig([pth '/plots/size.pdf']);

%% connectivity
cgroup = unique(conn(:));
cgroup(cgroup==0) = [];
x = [];
v = [];
v_cl = [];
v_cu = [];
w = [];
w_cl = [];
w_cu = [];for(i=1:length(cgroup))
	m = (vert==1000002) & (conn==cgroup(i)) & (regions==4) & (str==Str);
	x(i) = 2*cgroup(i);
	for j=1:length(solvers_legend)
		[y ci] = sim_eci(sweeps(m(:,5),j),0.3);
		v(i,j) = y;
		v_cl(i,j) = ci(1)-y;
		v_cu(i,j) = ci(2)-y;
		%
		[y ci] = sim_eci(work(m(:,5),j),0.3);
		w(i,j) = y;
		w_cl(i,j) = ci(1)-y;
		w_cu(i,j) = ci(2)-y;
	end
end

figure(1); clf;subplot(2,1,1);hold on;
for s=5:length(solvers_legend)
	errorbar(x',v(:,s),v_cl(:,s),v_cu(:,s),style{s});
end
h = legend(solvers_legend{5:end});
set(h,'Location','Best');
set(h, 'interpreter', 'latex');
ylim([0 inf]);
mxlim([min(x) max(x)]);
set(gca,'XTick',x);
xlabel('connectivity');
ylabel('number of sweeps');
%
%figure(2); clf;
subplot(2,1,2);hold on;
for s=1:length(solvers_legend)
	errorbar(x',w(:,s),w_cl(:,s),w_cu(:,s),style{s});
end
h = legend(solvers_legend);
set(h,'Location','Best');
set(h, 'interpreter', 'latex');
%mxlim([min(x) max(x)]);
ylim([0 inf]);
mxlim([min(x) max(x)]);
set(gca,'XTick',x);
xlabel('connectivity');
ylabel('CPU, sec.');
%
pdffig([pth '/plots/connectivity.pdf']);
%
%
%% split
rgroup = unique(regions(:));
rgroup(rgroup==0) = [];
x = [];
v = [];
v_cl = [];
v_cu = [];
w = [];
w_cl = [];
w_cu = [];
for(i=1:length(rgroup))
	m = (vert==1000002) & (regions==rgroup(i)) & (conn==4) & (str==Str);
	x(i) =rgroup(i);
	for j=1:length(solvers_legend)
		[y ci] = sim_eci(sweeps(m(:,5),j),0.3);
		v(i,j) = y;
		v_cl(i,j) = ci(1)-y;
		v_cu(i,j) = ci(2)-y;
		%
		[y ci] = sim_eci(work(m(:,5),j),0.3);
		w(i,j) = y;
		w_cl(i,j) = ci(1)-y;
		w_cu(i,j) = ci(2)-y;
	end
end
for j=1:4 %not region-based solvers_legend
	m = (vert==1000002) & (conn==4) & (str==Str);
	mean(work(m(:,5),j));
	[y ci] = sim_eci(work(m(:,5),j),0.3);	
	w(:,j) = y;
	w_cl(:,j) = ci(1)-y;
	w_cu(:,j) = ci(2)-y;	
end

figure(1); clf; subplot(2,1,1);hold on;
for s=5:length(solvers_legend)
	errorbar(x',v(:,s),v_cl(:,s),v_cu(:,s),style{s});
end
h = legend(solvers_legend{5:end});
set(h,'Location','Best');
set(h, 'interpreter', 'latex');
ylim([0 350]);
set(gca,'YTick',[30 100:50:350]);
mxlim([min(x) max(x)]);
set(gca,'XTick',x([1 3 5:length(x)]));
xlabel('# of regions');
ylabel('number of sweeps');
%
%figure(2); clf;
subplot(2,1,2);hold on;
ss = [1 2 3 4 5 6];
for s=ss%1:length(solvers_legend)
	errorbar(x',w(:,s),w_cl(:,s),w_cu(:,s),style{s}) & (str==s);
end
% h = legend(solvers_legend{ss});
% set(h,'Location','Best');
% set(h, 'interpreter', 'latex');
%mxlim([min(x) max(x)]);
%mxlim([0 inf]);
mxlim([min(x) max(x)]);
%set(gca,'XTick',x);
set(gca,'XTick',x([1 3 5:length(x)]));
ylim([0 inf]);
xlabel('# of regions');
ylabel('CPU, sec.');
%
pdffig([pth '/plots/regions.pdf']);
%% workload distribution
figure(2); clf;
subplot(1,2,1);
p = 1;
for s = 5:6
	m = any((vert==1000002) & (conn==4) & (str==Str) & regions==4,2);
	%m = any(regions==4,2);
	%i = [length(x)-3 length(x)];
	%i = i(p);
	subplot(1,2,p);
	bar(1:4,[mean(msg(m,s)) mean(augment(m,s)) mean(relabel(m,s)) mean(gap(m,s))],0.9);
	set(gca,'XMinorTick','off');
	set(gca,'XTick',1:4);
	set(gca,'XTickLabel',{'mgs','discharge','relabel','gap'});
	ylabel('time, sec.');
	%title(sprintf('Work distribution for N=%i',x(i)));
	legend(solvers_legend{s});
	p = p+1;
	box off;
end
pdffig([pth '/plots/workload.pdf']);
%%
% figure(2);clf; hold on;
% X = work(m(:,5),3);
% %ecdf(X);
% %hist(X,100);
% [f,x] = ecdf(X);
% ecdfhist(f,x,10);
% [y ci] = sim_eci(X,0.3);
% stem(y,1,'r');
% %ci = eci(X,0.3);
% stem(ci,[1,1],'y');
% [f,x,flo,fup] = ecdf(work(m(:,5),3));
% figure(2);clf;
% stairs(x,f,'LineWidth',2)
% hold on
% stairs(x,flo,'r:','LineWidth',2)
% stairs(x,fup,'r:','LineWidth',2)
% %%
% [m,p,g] = grpstats(work(m(:,5),3),1,{'mean','predci'});