function p = density_gmmd2_ml(nn,ro1,ro2,sigma1,sigma2,eps1)
%  density_gmmd2_ml(nn,ro1,ro2,sigma1,sigma2,eps1)
%  maximum likelihood density estimate 2D (very similar to Parzen window)
%	
%	nn - color samples
% ro1 - color distance function in dimention 1
% ro2 - color distance function in dimention 2

% sigma1 - variation of gaussian in dim1
% sigma2 - variation of gaussian in dim2
%
%

if ~exist('eps1','var') || isempty(eps1)
	eps1 = 1e-6;
end
	
	g1 = gauss_filter_d(ro1,sigma1);
	g2 = gauss_filter_d(ro2,sigma2);

	N = sum(nn(:));
	nn = nn/N;
	t = nn;
	%p = g1*nn*g2';
	%return;
	
	for i=1:5
		p = g1*t*g2';
		%fprintf('L=%f\n',MI_estimate(nn,p,eps1));% likelihood
		%imagesc(p);		
		z = nn; ii=p>eps1;
		z(ii)=z(ii)./p(ii);
		t = t.*(g1*z*g2');
		t = t/sum(t(:));
	end

end