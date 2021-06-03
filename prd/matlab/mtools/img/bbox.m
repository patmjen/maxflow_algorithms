function [l L] = bbox(mask)
% [l L] = bbox(mask) returns limits of mask

[i1 i2] = find(mask);

l = [min(i1(:)) min(i2(:))];
L = [max(i1(:)) max(i2(:))];

end