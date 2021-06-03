commandwindow;
run ../mtools/mpath.m
errors = {};
solvers={
	'BK',
	%'GT0',
	%'GT05',
	%'HPR',
	%'S-ARD',
	%'S-PRD',
	%'S-ARD1',
	'P-ARD1',
	%'P-PRD',
	%'P-DD'
	};

threads = [1,2,3,4,5,6,7,8];

%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/1-stereo/\BVZ-sawtooth/','BVZ-sawtooth0.max');

inst = dirr('~/datagrid/dev/data/d_maxflow/vision/2-multiview/','*.max');
%inst = [inst dirr('~/datagrid/dev/data/d_maxflow/vision/3-surface/','*.max')];
%inst = [inst dirr('~/datagrid/dev/data/d_maxflow/vision/4-segm/','*.max')];

%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/1-stereo/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/2-multiview/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/3-surface/','*.max');
%inst = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/4-segm/','*.max');

for i=1:length(inst)
	fprintf('problem:  %s\n',inst{i});
	%for nR=[2 4]
	for j=1:length(solvers)
		f = dir(inst{i});
		if(f.bytes>1024^3*12)
			continue;
		end
		parallel = false;
		if(strcmp(solvers{j}(1:2),'P-'))
			parallel = true;
			Threads = threads;
		else
			Threads = 1;
		end
		for t = Threads
			solver = solvers{j};
			
			if(parallel || strcmp(solvers{j}(1:2),'S-'))
				solver = [solver '-t' num2str(t)];
			end
			name = [inst{i} '.' solver];
			%         if(strcmp(solvers{j},'S-ARD') && exist(name,'file'))
			%             movefile(name, [name '.preproc']);
			%         end
			%{
       if(strcmp(solvers{j},'S-ARD1') && exist(name,'file'))
            delete(name);
            %movefile(name, [name '.old']);
       end
       continue;
			%}
			if(~exist([name '.sol'],'file'))
				fprintf('solver:  %s\n',solver);
				system([sys_slashes('../../bin/d_maxflow/solve_dimacs') ' ' solver ' ' inst{i}]);
				status = system([sys_slashes('../../bin/d_maxflow/cut_cost') ' ' inst{i} ' ' name '.cut']);
				if(status~=0)
					errors{end+1} = name;
					fprintf('************problem detected*************************\n');
					%error('problem detected');
				end
				%return;
			end
		end
	end
	%end
end
if(length(errors)>0)
	fprintf('ERRORS:\n');
	for i=1:length(errors)
		fprintf('%s\n',errors{i});
	end
end