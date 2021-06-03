function imagesc_j(I)

I = I-min(I(:));
m = max(I(:));
if(m>1)
	I = I/m;
end


if(size(I,3)==1)
    mask = isnan(I);
    I = ind2rgb(floor(I*64),jet);
    for i=1:size(I,3)
        a = I(:,:,i);
        a(mask) = nan;
        I(:,:,i) = a;
    end
end

I = color_nans(I,[0,0,0]);

imagesc(I);

end