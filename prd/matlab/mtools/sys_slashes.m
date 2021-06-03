function pth = sys_slashes(pth)

if(~isempty(strfind(lower(computer()),'win')))
	pth = win_slashes(pth);
else
	pth = unix_slashes(pth);
end

end
