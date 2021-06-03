function C = msub(A,B,dim);
%  C = msub(A,B,dim) subtract B from A on dimension dim
%		B is one-dimensional
%
%

sz = size(A);
nsz = sz;
sz(dim) = 1;
nsz(1:length(sz)~=dim)=1;
C = A - repmat(reshape(B,nsz),sz);	
end