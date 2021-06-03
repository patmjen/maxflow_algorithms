%function rrename(pth,mask,pattern,replace)
commandwindow;
pth = 'G:/personal/shekhovt/dev/data/d_maxflow/vision/';
mask = '*.x4x4x4.*';
pattern = '.x4x4x4';
replace = '';

ff = dirr(pth,mask);
for i=1:length(ff)
	a = ff{i};
	p = strfind(a,pattern);
	if(~isempty(p))
		fprintf('%s -> ',file_name(a));
		p = p(1);
		b = [a(1:p-1) replace a(p+length(pattern):end)];
		fprintf('%s\n',file_name(b));
		%movefile(a,b);
		system(['ren ' win_slashes(a) ' ' file_name(b)]);
	end
end

%end