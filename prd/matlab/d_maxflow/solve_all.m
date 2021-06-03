commandwindow;
run ../mtools/mpath.m
errors = {};
solvers={
    'BK',
    'GT0',
    'GT05',
    'HPR',
    'S-ARD',
    'S-PRD',
    'S-ARD1',
    'P-ARD1',
    'P-PRD',
    'P-DDx2',
    'P-DDx4',
    };

inst = dirr('~/datagrid/dev/data/d_maxflow/vision/2-multiview/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/1-stereo/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/2-multiview/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/3-surface/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/4-segm/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/4-segm/5-abdomen/','*.max');

for i=1:length(inst)
    fprintf('problem:  %s\n',inst{i});
    for j=1:length(solvers)      
        if(strcmp(solvers{j}(1:2),'P-'))% solver is parallel
            f = dir(inst{i});
            if(f.bytes>1024^3*2)%too large, dont process it with this solver
                continue;
            end
        end       
        name = [inst{i} '.' solvers{j} '.sol'];
        %         if(strcmp(solvers{j},'S-ARD') && exist(name,'file'))
        %             movefile(name, [name '.preproc']);
        %         end
        %{
       if(strcmp(solvers{j},'S-ARD1') && exist(name,'file'))
            %delete(name);
            movefile(name, [name '.old2']);
       end
       continue;
        %}
        if(~exist(name,'file'))
            fprintf('solver:  %s\n',solvers{j});
            system(['../../bin/d_maxflow/solve_dimacs ' solvers{j} ' ' inst{i}]);
            status = 0;%system(['../../bin/d_maxflow/cut_cost ' inst{i} ' ' inst{i} '.' solvers{j} '.cut']);
            if(status~=0)
                errors{end+1} = name;
                fprintf('************problem detected*************************\n');
                %error('problem detected');
            end
        end
    end
end
if(length(errors)>0)
    fprintf('ERRORS:\n');
    for i=1:length(errors)
        fprintf('%s\n',errors{i});
    end
end