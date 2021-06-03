function str = lstrtrim(str,trimstr)
	l = findstr(str,trimstr);
	str = str(l+length(trimstr):end);
end