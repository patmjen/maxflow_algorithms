function [J l L]  = imcrop(I,l,L);
%
% [J l L]  = imcrop(I,l,L) returns cut-out from image I and refined box if out of bounds
%

l = max(l,1);
L = min(L,reshape([size(I,1) size(I,2)],size(L)));
J = I(l(1):L(1),l(2):L(2),:);

end