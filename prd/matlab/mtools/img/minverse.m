function g = minverse(f,mask)

a = mmeshgrid(size(f));
f2 = f(:,:,2);
f1 = f(:,:,1);
for i=1:size(f,3)
	aa = a(:,:,i);
	g(:,:,i) = griddata(f2(mask),f1(mask),aa(mask),a(:,:,2),a(:,:,1),'linear');
end
m = isnan(g);
g(m)=a(m);
end