function C = conv2_gauss(A, sigma)
%
%  C = conv2_gauss(A, sigma)
%
if(sigma>0)
	b = normpdf([0:1:max(2,3*sigma)],0,sigma);
	b = [fliplr(b(2:end)) b];
	b = b/sum(b);
	for i=1:size(A,3)
		C(:,:,i) = conv2(A(:,:,i),b,'same');
		C(:,:,i) = conv2(C(:,:,i),b','same');
	end
else
	C = A;
end
end