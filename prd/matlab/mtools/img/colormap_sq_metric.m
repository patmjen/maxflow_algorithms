function d = colormap_sq_metric(map)
n = size(map,1);
map = reshape(map,n,1,[]);
a = repmat(map,1,n);
d = a-permute(a,[2 1 3]);
d = sum(d.^2,3);
end