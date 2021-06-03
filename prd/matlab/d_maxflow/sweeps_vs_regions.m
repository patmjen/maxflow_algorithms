commandwindow;
errors = {};
solvers={
    'S-ARD1',
    };

inst ={
    %'G:/personal/shekhovt/dev/data/d_maxflow/vision/2-multiview/BL06-gargoyle-sml.max',
    'G:/personal/shekhovt/dev/data/d_maxflow/vision/3-surface/LB07-bunny-med.max',
    'G:/personal/shekhovt/dev/data/d_maxflow/vision/4-segm/1-liver/liver.n6c10.max',
    };
split1 = num2cell([1:64]);
split2 = {
    1,
    [2 1 1],
    [3 1 1],
    [2 2 1],
    [3 2 1],
    [2 2 2],
    [3 2 2],
    [3 3 2],
    [3 3 3],
    [4 2 2],
    [4 3 3],
    [4 4 3],
    [4 4 4],
    };

split = split2;

for i=1:length(inst)
    fprintf('problem:  %s\n',inst{i});
    for j=1:length(split)
        solver = [solvers{1}];
        for l=1:length(split{j})
            solver = [solver 'x' num2str(split{j}(l))];
        end
        name = [inst{i} '.' solver '.sol'];
        if(~exist(name,'file'))        
            cache = [inst{i} '_reg'];
            try
            rmdir(cache,'s');
            catch
            end
            fprintf('solver:  %s\n',solver);
            system(['..\tests\tests.exe ' solver ' ' inst{i}]);
            status = system(['..\cut_cost ' inst{i} ' ' inst{i} '.' solver '.cut']);
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