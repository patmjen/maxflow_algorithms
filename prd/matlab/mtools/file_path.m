function pth = file_path(f)

	[pathstr, name, ext, versn] = fileparts(f);
	pth =  pathstr;

end