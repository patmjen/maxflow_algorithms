function medit(a)
% opens all files in the given directory for edditing
% for class opens all methods of the class

if(~ischar(a))
	help medit;
	error('Expecting string on input.');
end

%try @a
if a(1)~= '@' && exist(['@' a],'file')
	medit(['@' a]);
else
	if exist(a,'file')
		if exist(a,'dir')
			files = dir([a '/*.m']);
			for i=1:length(files)
				name = files(i).name;
				if strcmp(name,'display.m') || strcmp(name,'subsasgn.m') || strcmp(name,'subsref.m')
					continue;
				end
				ff = [a '/' name];
				edit(ff);
				%fprintf('%s\n',ff);
			end
		else
			edit(a);
		end
	else
		error('Cant find ''%s'' on the path or dont know what to do with it.', a);
	end
end

end