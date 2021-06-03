function ext = platform_shared_lib_ext()

if(~isempty(strfind(lower(computer()),'win')))
	ext = 'lib';
else
	ext = 'a';
end

end