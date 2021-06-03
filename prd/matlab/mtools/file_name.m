function r = file_name(f)

	[pathstr, name, ext, versn] = fileparts(f);
	r =  [name ext];

end