commandwindow;
run ../../matlab/mtools/mpath.m

inst ={
    [data() 'vision/2-multiview/BL06-gargoyle-lrg.max'],
    [data() 'vision/2-multiview/BL06-camel-lrg.max'],
    [data() 'vision/3-surface/LB07-bunny-lrg.max'],
    [data() 'vision/4-segm/1-liver/liver.n26c100.max'],
%   [data() 'vision/4-segm/2-babyface/babyface.n6c100.max'],
%   [data() 'vision/4-segm/3-adhead/adhead.n6c10.max'],
%   [data() 'vision/4-segm/4-bone/bone.n6c10.max'],
    };

P = {};

solvers = {'P-ARD1',
    %'P-PRD'
    };

P=[];

for ss=1:length(solvers)
    for i=1:length(inst)
        ff = dirr([file_path(inst{i}) '/'], [file_name(inst{i}) '.' solvers{ss} '*-t*.sol']);
        R = {};
        P{i,ss} = [];
        for j=1:length(ff)
            r = read_report(ff{j});
            R{j} = r;
            P{i,ss}(1,j) = R{j}.threads;
            P{i,ss}(2,j) = R{j}.gap+R{j}.relabel+R{j}.msg+R{j}.augment;
            P{i,ss}(3,j) = R{j}.sweeps;
        end
        [ans p] = sort(P{i,ss}(1,:));
        P{i,ss} = P{i,ss}(:,p);
    end
end

%%
figure(1); clf; subplot(1,1,1); hold on;
cc = {'r.-','go-','bs-','k.-','mo-','cs-','y.-';
      'r.:','go:','bs:','k.:','mo:','cs:','y.:'}';
for ss=1:length(solvers)
    for i=1:length(P)
        plot(P{i,ss}(1,:),P{i,ss}(2,:)/P{i,ss}(2,1),cc{i,ss});
    end
end
ll={};
for i=1:length(inst)
    ll{i} = trim_ext(file_name(inst{i}));
end
%legend({'BL06-gargoyle-sml','LB07-bunny-med','liver.n6c10'});
%xlim([1 64]);
%set(gca,'XTick',[1 4 8 16 32 64]);
xlabel('#CPUs');
ylabel('Relative Time');
%
h = legend(ll,'Location','NorthEast');
%
return
subplot(2,1,2); hold on;
%cc = {'r.-','go-','bs-'};
for ss=1%1:length(solvers)
    for i=1:length(P)
        plot(P{i,ss}(1,:),P{i,ss}(3,:),cc{i,ss});
    end
end
%
h = legend(ll,'Location','SouthEast');
%set(h, 'interpreter', 'latex');
%xlim([1 64]);
%ylim([1 inf]);
%set(gca,'XTick',[1 4 8 16 32 64]);
%set(gca,'YTick',[1 5:5:25]);
xlabel('#CPUs');
ylabel('P-ARD sweeps');
%
%pdffig(['../../../tex/mixed_maxflow/fig/sweeps_vs_regions.pdf']);