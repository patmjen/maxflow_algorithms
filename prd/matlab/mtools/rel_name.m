function r = rel_name(pth,full_name)
full_name = path_reduce(full_name);
i = strmatch(pth, full_name);
if(~isempty(i))
	r = full_name(length(pth)+1:end);
else
	r = full_name;
end
end