function c = cross2(a,b)

switch ndims(a)
case 2
	c = a(1,:).*b(2,:)-a(2,:).*b(1,:);
case 3
	c = a(1,:,:).*b(2,:,:)-a(2,:,:).*b(1,:,:);
end

end