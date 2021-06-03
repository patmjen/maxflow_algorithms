function force_path(file)

p = file_path(file);

if ~exist(p,'dir')
	mkdir(p);
end

end