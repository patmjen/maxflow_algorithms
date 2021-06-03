function x = pad_bd_copy(x,k)

for i=1:k
	x = cat(1,x(1,:,:),x,x(end,:,:));
	x = cat(2,x(:,1,:),x,x(:,end,:));
end

end