function str = rstrtrim(str,trimstr)
	str = fliplr(lstrtrim(fliplr(str),fliplr(trimstr)));
end