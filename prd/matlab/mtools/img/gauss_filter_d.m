function g = gauss_filter_d(d,sigma)
% g = gauss_filter_d(d,sigma)
%
%	Gauss filter for metric function d of variance sigma
%
%
	n = size(d,1);
	d_th = (10*sigma)^2; %% truncate non-importent part
	%ii = d<d_th;
	ii = d<100;
	g = zeros(n,n);
	g(ii) = 1/(sqrt(2*pi)*sigma)*exp(-d(ii)/(2*sigma^2));
	g = g./repmat(sum(g,1),n,1);
end