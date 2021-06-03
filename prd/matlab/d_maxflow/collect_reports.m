if(~exist('files','var'))
	R = {};
	files = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/','*.sol');
%	
	for i=1:length(files)
		r = read_report(files{i});
        R{i} = r;
	end
end

for i=1:length(R)
    r = R{i};
    if isfield(r,'disk_writ')
        if ~isfield(r,'disk_wb') || isempty(r.disk_wb) || isequal(r.disk_wb,0)
            r.disk_wb = r.disk_writ*2*33;%*line{5}.disk/line{5}.disk_writ;
        end
    end
    R{i} = r;
end

%fprintf(f,'\\hline\n');
%fprintf(f,'\\rowcolor{blue!10}\n');
name = '';
type = '';
vert = 0;
edges = 0;
lines = cell(0,7);
i = 1
%while i<= length(files)+1
for i=1:length(files)+1
	if i>length(files) || ~strcmp(name,R{i}.name)
		if(i>1)				
			lines(end+1,1:end) = line;
			lines{end,end}.name = name;
			lines{end,end}.vert = vert;
			lines{end,end}.edges = edges;
			lines{end,end}.type = type;
			% 			e = size(lines,1)+1;
			% 			for k=1:length(solvers)
			% 				lines{e,k} = line{k};
			% 			end
			if(i>length(files))
				break
			end
		end
		name = R{i}.name;
		type = R{i}.type;
		line = cell(1,6);
		vert = 0;
		edges = 0;
		line{7}.source_size = R{i}.source_size;
	end
	if ~isfield(R{i},'solver')
		line{7} = R{i};
		continue;
	end
	%solver = R{i}.solver;
	for k=1:length(solvers)
		if(strcmp(R{i}.solver,solvers{k}))
			line{k} = R{i};
			if(isfield(R{i},'msg') && isfield(R{i},'augment') && isfield(R{i},'relabel') && isfield(R{i},'gap') )
				line{k}.cpu = R{i}.msg+R{i}.augment+R{i}.relabel+R{i}.gap;
			elseif isfield(R{i},'augment')
				line{k}.cpu = R{i}.augment;
			end
			if(k<5)
				if(isfield(R{i},'solve'))
					line{k}.cpu = R{i}.solve;
				else
				end
			end
			if(isfield(R{i},'sweeps'))
				line{k}.sweeps = R{i}.sweeps;
			end
			if isfield(R{i},'disk_rb')
				if(isscalar(R{i}.disk_rb))
					line{k}.disk = R{i}.disk_rb+R{i}.disk_wb;
				else
					line{k}.disk = R{i}.disk_rb;
				end
			end
			%if(k==6)
			%	line{k}.disk = R{i}
			%end
			%if(R[i].vert>0)
			%end
			if(k==1 || (vert==0 && k==5))
				vert = R{i}.vert;
				edges = R{i}.edges;
			end
		end
	end
end

k=1;
%sum up stereo subproblems
N=1;
while(k<size(lines,1))
	l1 = lines(k,:);
	l2 = lines(k+1,:);
	if ~isempty(l1{1}) && ~isempty(l2{1}) && strcmp(l1{1}.type,'1-stereo') && strcmp(l1{1}.name(1:6),l2{1}.name(1:6))
		for j=1:length(solvers)
			names = fieldnames(struct(l1{j}));
			for n=1:length(names)
				if isscalar(l1{j}.(names{n})) && isfield(l2{j},names{n}) && isscalar(l2{j}.(names{n}))
					l1{j}.(names{n}) = l1{j}.(names{n}) + l2{j}.(names{n});
				end
			end
		end
		lines(k,:) = l1;
		lines(k+1,:) = [];
		N = N+1;
	else
		for j=1:length(solvers)
			if ~isempty(l1{j})
				names = fieldnames(l1{j});
				for n=1:length(names)
					if isscalar(l1{j}.(names{n}))
						if ~isempty(strmatch(names{n},{'sweeps','mem_shared','mem_region','disk','Regions','preprocess'}))
							l1{j}.(names{n}) = l1{j}.(names{n})/N;
						end
					end
				end
			end
		end
		if N>1
			l1{end}.name = [l1{end}.name(1:end-1) '(' num2str(N) ')'];
			%vert = [num2str(N) 'x' sprintf(vert,'%3.1f')];
		end
		lines(k,:) = l1;
		k = k+1;
		N = 1;
	end
end
% for j=1:length(solvers)
% 	if ~isempty(l1{j})
% 		names = fieldnames(l1{j});
% 		for n=1:length(names)
% 			if isscalar(l1{j}.(names{n}))
% 				l1{j}.(names{n}) = l1{j}.(names{n})/N;
% 			end
% 		end
% 	end
% end
if N>1
	l1{end}.name = l1{end}.name(1:end-1);
end
%lines(k,:) = l1;
