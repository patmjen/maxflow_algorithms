function res = dirr(pth,mask)
res = {};
ff = dir([pth mask]);
for i=1:length(ff)
	res{end+1} = [pth ff(i).name];		
end

allf = dir(pth);
for i=1:length(allf)
	if(allf(i).isdir && ~strcmp(allf(i).name,'.') && ~strcmp(allf(i).name,'..'))
		res = [res dirr([pth allf(i).name '/'],mask)];
	end
end

end