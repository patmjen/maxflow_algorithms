function lines = get_reports(pth,solvers)

files = dirr(pth,'*.sol');
%files = dirr('G:\personal\shekhovt\dev\data\d_maxflow\vision/1-stereo/BVZ-sawtooth/','*.sol');
%G:\personal\shekhovt\dev\data\d_maxflow\vision\4-segm\5-abdomen\

R = {};

for i=1:length(files)
	r = read_report(files{i});
%	p = strfind(r.name,'_inst');
%	r.name = r.name(1:p-1);
	R{i} = r;
end

for i=1:length(R)
	R{i}.name = trim_ext(R{i}.name);
end
%fprintf(f,'\\hline\n');
%fprintf(f,'\\rowcolor{blue!10}\n');
name = '';
type = '';
vert = 0;
edges = 0;
lines = cell(0,length(solvers)+1);
i = 1
%while i<= length(files)+1
for i=1:length(files)+1
	if i>length(files) || ~strcmp(name,R{i}.name)
		if(i>1)
			if isfield(line{6},'disk_writ')
				if ~isfield(line{6},'disk') || isempty(line{6}.disk)
					line{6}.disk = line{6}.disk_writ*2*33;%*line{5}.disk/line{5}.disk_writ;
				end
				if ~isempty(line{6}.disk) && isequal(line{6}.disk,0)
					line{6}.disk = line{6}.disk_writ*2*33;%*line{5}.disk/line{5}.disk_writ;
				end
			end
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
		continue;
	end
	%solver = R{i}.solver;
	for k=1:length(solvers)
		if(strcmp(R{i}.solver,solvers{k}))
			line{k} = R{i};
			if(isfield(R{i},'msg') && isfield(R{i},'augment') && isfield(R{i},'relabel') && isfield(R{i},'gap') )
				line{k}.cpu = R{i}.msg+R{i}.augment+R{i}.relabel+R{i}.gap;
			end
			if(k<5)
				line{k}.cpu = R{i}.solve;
			end
			line{k}.sweeps = R{i}.sweeps;
			if(isscalar(R{i}.disk_rb))
				line{k}.disk = R{i}.disk_rb+R{i}.disk_wb;
			else
				line{k}.disk = R{i}.disk_rb;
			end
			%if(k==6)
			%	line{k}.disk = R{i}
			%end
			%if(R[i].vert>0)
			%end
			if(k==1 || k==5)
				vert = R{i}.vert;
				edges = R{i}.edges;
			end
		end
	end
end

return;
k=1;
%sum up stereo subproblems
N=1;
while(k<size(lines,1))
	l1 = lines(k,:);
	l2 = lines(k+1,:);
	if ~isempty(l1{1}) && ~isempty(l2{1}) && strcmp(l1{1}.name,l2{1}.name)
		for j=1:length(solvers)
			names = fieldnames(l1{j});
			for n=1:length(names)
				if isscalar(l1{j}.(names{n})) && isscalar(l2{j}.(names{n}))
					l1{j}.(names{n}) = l1{j}.(names{n}) + l2{j}.(names{n});
					if(~isfield(l1{j},[names{n}+'_std']))
						l1{j}.([names{n}+'_std']) = 0;
					end
					l1{j}.([names{n}+'_std']) = l1{j}.([names{n}+'_std']) + l2{j}.(names{n})^2;
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
						l1{j}.(names{n}) = l1{j}.(names{n})/N;
						l1{j}.([names{n} '_std']) = sqrt(l1{j}.([names{n} + '_std'])/N - l1{j}.(names{n})^2);
					end
				end
			end
		end
		if N>1
			l1{end}.name = l1{end}.name(1:end-1);
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
end