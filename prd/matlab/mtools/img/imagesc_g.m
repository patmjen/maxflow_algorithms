function mimagesc(I)

I = I-min(I(:));
m = max(I(:));
if(m>1)
	I = I/m;
end
imagesc(I);

end