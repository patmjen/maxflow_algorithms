commandwindow;
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/','*.max');
inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/','*.max_reg');
for i=1:length(inst)
    %dd = [inst{i} '_reg/'];
    dd = inst{i};
    if(exist(dd,'dir')==7)
        fprintf('clearing:  %s\n',dd);
        rmdir(dd,'s');
    end
end
    