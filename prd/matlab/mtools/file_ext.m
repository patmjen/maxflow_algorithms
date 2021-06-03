function r = file_name(f)

	[pathstr, name, ext, versn] = fileparts(f);
	r =  ext;

end