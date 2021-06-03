function g = mgriddata(cp1,cp2,f,method)
%
%	g = (cp1,cp2,f,method)
%	
%		d [n1 x n2 x 2]
%		I [m1 x m2 x m3]
%
%		method = ['nearest' 'linear' 'spline' 'cubic']
%
%
%g = mmeshgrid(size(d));
for i=1:size(I,3)
	a = f(:,:,i);
	ii = sub2ind(size(a),cp2(1,:),cp2(2,:));
	g(:,:,i) = griddata(cp2(2,:),cp2(1,:),a(ii),g(:,:,2),g(:,:,1),method);
end

end