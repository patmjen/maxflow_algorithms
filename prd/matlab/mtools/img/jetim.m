function I = jetim(A,c)
if ~exist('c','var') || isempty(c)
	a = min(A(:));
	b = max(A(:));
	m1 = min(a,-b);
	m2 = max(b,-a);
else
	m1 = c(1);
	m2 = c(2);
	A = max(A,m1);
	A = min(A,m2);
end
	J = jet;
	Ai = floor((A-m1)/(m2-m1)*63)+1;
	I = J(Ai,:);
	I = reshape(I,[size(A,1) size(A,2) 3]);

end