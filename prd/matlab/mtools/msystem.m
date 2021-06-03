function status = msystem(command)

if ispc
	status = system(command);
else
	status = system(unix_slashes(command));
end
end