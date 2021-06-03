function hh = histi(I1,n1) 
% hh = histi(I1,n1)  -- discrete histogram
%
% n1 must match max(I1(:))

n = length(I1(:));
hh = full(sparse(ones(n,1),double(I1(:)),ones(n,1),1,n1));

end