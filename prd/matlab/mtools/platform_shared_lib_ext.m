function ext = platform_shared_lib_ext()

if(~isempty(strfind(lower(computer()),'win')))
	ext = 'dll';
else
	ext = 'so';
end

end