function R = read_report(filename)

R = [];
name = file_name(filename);
p = strfind(name,'.max');
if(isempty(p))
	p = strfind(name,'.sol');
end
Solver = trim_ext(name(p+5:end));
name = name(1:p-1);

[ss params] = strtok(Solver(3:end),{'.-x'});
Solver = [Solver(1:2) ss];
if ~isempty(params)
    if(length(params)>0 && params(1)=='x')
        [p1 params] = strtok(params(2:end),{'.-x'});
    end
    if(length(params)>0 && params(1)=='-')
        [p1 params] = strtok(params(2:end),{'-'});
        if(length(p1)>0)
            switch p1(1)
                case 't'
                    R.('threads') = sscanf(p1(2:end),'%i');
            end
        end
    end
    
end

R.name = name;
pth = file_path(filename);
p = strfind(pth,'vision');
[a rem] = strtok(pth(p:end),'/');
R.type = strtok(rem,'/');
R.regulargrid = [];
try
    source = [trim_ext(filename) '.max'];
    d = dir(source);
    R.source_size = d.bytes;
	f = fopen(source,'r');
		while(~feof(f))
			l = fgetl(f);
			if(isempty(l))
				continue;
			end
			if l(1)=='a'
				break
			else
				l = l(2:end);
				s1 = sscanf(l,' %s ',1);
				if strcmp(s1,'regulargrid') || strcmp(s1,'complexgrid')
					R.regulargrid = sscanf(l,' %*s %i %i %i %i');
					break;
				end
				if strcmp(s1,'Size:')
					R.regulargrid = sscanf(l,' %*s %i%*c%i%*c%i%*c%i');
					break;
				end
			end
		end
	fclose(f);
catch
    source = trim_ext(trim_ext(filename));
    d = dir(source);
	if(~isempty(d))
		R.source_size = d.bytes;
	else
		R.source_size = 0;
	end
end

f = fopen(filename,'r');
while ~feof(f)
	l = fgetl(f);
	if(l==-1)
		break;
	end
	[tok remain] = strtok(l,':');
	tok = strrep(strtrim(tok),' ','_');
	if(isempty(remain))
		continue;
	end;
	if(strcmp(tok,'split'))
		v = strread(remain(2:end),' %f');
	else
		[tok1 remain] = strtok(remain,': ');
		[v c] = sscanf(tok1,'%f');
		if(c==0)
			[v c] = sscanf(tok1,'%s');
		end
		% 	if strcmp(tok,'split')
		% 		tok = 'Regions';
		% 	end
	end
	tok = strrep(strrep(tok,',',''),' ','_');
	R.(tok) = v;
	if strcmp(tok,'disk_read')
		[tok2 remain] = strtok(remain,'/ ');
		[v c] = sscanf(tok2,'%f');
		if(c==0)
			[v c] = sscanf(tok2,'%s');
		end
		R.('disk_rb') = v;
	end
	if strcmp(tok,'disk_writ')
		[tok2 remain] = strtok(remain,'/ ');
		[v c] = sscanf(tok2,'%f');
		if(c==0)
			[v c] = sscanf(tok2,'%s');
		end
		R.('disk_wb') = v;
	end
	if strcmp(tok,'sweep') && str2num(tok1)==0
		%: 0 flow: 12185842 decided:  8.9%
		%sweep = sscanf(remain,'%*c %i',1);
		%if sweep==1
		preprocess =  sscanf(remain,'%*s %*f %*s %f',1);
		R.('preprocess') = preprocess;
		%end
	end
	%[A count] = sscanf(l,'%s',1);
	%k = strfind(l,':');
	%s = l(1:k-1);
	%R = setfield(R,s,0);
	%[A count] = sscanf(l,' %s');
end

if(~isfield(R,'Regions') && isfield(R,'split'))
	R.('Regions') = prod(R.split);
	%R = rmfield(R,'split');
end

if(~isempty(Solver))
	R.solver = Solver;
end

fclose(f);
end

function s= deblanc(s)
k = strfind(l,':');
s = l(1:k-1);
end