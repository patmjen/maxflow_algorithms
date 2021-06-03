function C = conv1_gauss(A, sigma)
%
%  C = conv2_gauss(A, sigma)
%
if(sigma>0)
	b = normpdf([0:1:max(2,3*sigma)],0,sigma);
	b = [fliplr(b(2:end)) b];
	b = b/sum(b);
	for i=1:size(A,3)
		if(size(A,2)==1)
			C(:,:,i) = conv2(A(:,:,i),b','same');
		else
			C(:,:,i) = conv2(A(:,:,i),b,'same');
		end
	end
else
	C = A;
end
end