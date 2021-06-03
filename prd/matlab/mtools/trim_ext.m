function r = trim_ext(f)
	[pathstr, name, ext] = fileparts(f);
	if(~isempty(pathstr))
		r =  [pathstr '/' name];
	else
		r =  name;
	end
end