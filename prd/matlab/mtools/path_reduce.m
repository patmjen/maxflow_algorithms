function a = path_reduce(a)

a = strrep(a,'\','/');
while ~isempty(strfind(a,'//'))
	a = strrep(a,'//','/');
end

end