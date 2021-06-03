function pth = data()
if(~isempty(strfind(lower(computer()),'win')))
	pth = 'G:\personal/shekhovt/dev/data/d_maxflow/';
else
	pth = '../../data/d_maxflow/';
end
end