function [g g_mask] = minterp2(d,f,method)
%
%
%		method = ['nearest' 'linear' 'spline' 'cubic']
%
%
a = mmeshgrid(size(f));
if islogical(f)
	for i=1:size(f,3)
		g(:,:,i) = interp2(a(:,:,2),a(:,:,1),f(:,:,i),d(:,:,2),d(:,:,1),'nearest',0);
	end
else
	for i=1:size(f,3)
		g(:,:,i) = interp2(a(:,:,2),a(:,:,1),f(:,:,i),d(:,:,2),d(:,:,1),method,nan);
	end
	m = isnan(g);
	g(m) = 0;
	g_mask = ~any(m,3);
end

end