function p = density_gmmd1_ml(nn,ro1,sigma1,eps1)
%  density_gmmd1_ml(nn,ro1,sigma1,eps1)
%  maximum likelihood density estimate (very similar to Parzen window)
%	
%	nn - color samples
% ro1 - color distance function

% sigma1 - variation of gaussian
%

if ~exist('eps1','var') || isempty(eps1)
	eps1 = 1e-6;
end
    nn = nn(:);	
	g1 = gauss_filter_d(ro1,sigma1);

	N = sum(nn(:));
	nn = nn/N;
	t = nn;
	% Usual Parzen estimate:
	p = g1*nn;
    p = p/sum(p);
	%return;
	
	for i=1:5
		p = g1*t;
		%fprintf('L=%f\n',MI_estimate(nn,p,eps1));% likelihood
		%imagesc(p);		
		z = nn; ii=p>eps1;
		z(ii)=z(ii)./p(ii);
		t = t.*(g1*z);
		t = t/sum(t(:));
	end
end