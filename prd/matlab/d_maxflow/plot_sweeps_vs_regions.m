commandwindow;
run ../../matlab/mtools/mpath.m
inst ={
    'G:/personal/shekhovt/dev/data/d_maxflow/vision/2-multiview/BL06-gargoyle-sml.max',
    'G:/personal/shekhovt/dev/data/d_maxflow/vision/3-surface/LB07-bunny-med.max',
    'G:/personal/shekhovt/dev/data/d_maxflow/vision/4-segm/1-liver/liver.n6c10.max',
    };

P = {};

for i=1:length(inst)
    ff = dirr([file_path(inst{i}) '/'], [file_name(inst{i}) '.S-ARD1x*.sol']);
    R = {};
    P{i} = [];
    for j=1:length(ff)
        r = read_report(ff{j});
        R{j} = r;
        P{i}(1,j) = R{j}.Regions;        
        P{i}(2,j) = R{j}.gap+R{j}.relabel+R{j}.msg+R{j}.augment;
        P{i}(3,j) = R{j}.sweeps;
    end
    [ans p] = sort(P{i}(1,:));
    P{i} = P{i}(:,p);
end

%%
figure(1); clf; subplot(2,1,1); hold on;
cc = {'r.-','go-','bs-'};
for i=1:length(P)
    plot(P{i}(1,:),P{i}(2,:),cc{i});
end
%legend({'BL06-gargoyle-sml','LB07-bunny-med','liver.n6c10'});
xlim([1 64]);
set(gca,'XTick',[1 4 8 16 32 64]);
xlabel('Regions');
ylabel('Time, s.');
%
subplot(2,1,2); hold on;
cc = {'r.-','go-','bs-'};
for i=1:length(P)
    plot(P{i}(1,:),P{i}(3,:),cc{i});
end
h = legend({'BL06-gargoyle-sml','LB07-bunny-med','liver.n6c10'},'Location','SouthEast');
%set(h, 'interpreter', 'latex');
xlim([1 64]);
%ylim([1 inf]);
set(gca,'XTick',[1 4 8 16 32 64]);
set(gca,'YTick',[1 5:5:25]);
xlabel('Regions');
ylabel('Sweeps');
%
pdffig(['../../../tex/mixed_maxflow/fig/sweeps_vs_regions.pdf']);