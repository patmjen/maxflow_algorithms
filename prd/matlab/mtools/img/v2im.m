function I = v2im(v,sz)

v = transp(v);
S = size(v);
S = [sz S(2:end)];
I = reshape(v,S);

end