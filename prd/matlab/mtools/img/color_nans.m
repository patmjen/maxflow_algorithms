function A = color_nans(A,color)

if(~exist('color','var') || isempty(color))
color = [1 1 0];
end

mask = any(isnan(A),3);
for i=1:size(A,3)
    a = A(:,:,i);
    a(mask) = color(i);
    A(:,:,i) = a;
end

end