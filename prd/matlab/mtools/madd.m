function C = madd(A,B,dim);
sz = size(A);
nsz = sz;
sz(dim) = 1;
nsz(1:length(sz)~=dim)=1;
C = A + repmat(reshape(B,nsz),sz);	
end