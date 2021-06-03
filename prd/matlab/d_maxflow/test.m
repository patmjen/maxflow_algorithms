commandwindow;
run ../mtools/mpath.m
errors = {};
solvers={
	'BK',
	'GT0',
	'GT05',
	%'HPR',
	'S-ARD',
 	'S-PRD',
 	'S-ARD1',
 	'P-ARD1',
 	'P-PRD',
 	'P-DDx2',
 	'P-DDx4',
	};

inst = dirr('../../test/d_maxflow/','*.max');
for i=1:length(inst)
	fprintf('problem:  %s\n',inst{i});
	for j=1:length(solvers)
		name = [inst{i} '.' solvers{j} '.sol'];
		fprintf('solver:  %s\n',solvers{j});
		status = msystem(['..\..\bin\d_maxflow\solve_dimacs-D.exe ' solvers{j} ' ' inst{i}]);
		status = status | msystem(['..\..\bin\d_maxflow\cut_cost-D.exe ' inst{i} ' ' inst{i} '.' solvers{j} '.cut']);
		if(status~=0)
			errors{end+1} = name;
			fprintf('************problem detected*************************\n');
			%error('problem detected');
		end
	end
end
%
if(length(errors)>0)
	fprintf('ERRORS:\n');
	for i=1:length(errors)
		fprintf('%s\n',errors{i});
	end
end