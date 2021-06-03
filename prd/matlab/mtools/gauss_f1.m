function C = gauss_f1(A, sigma)
%
%
if(sigma>0)
	b = normpdf([0:1:max(2,3*sigma)],0,sigma);
	b = [fliplr(b(2:end)) b];
	b = b/sum(b);
	C = conv2(A,b','same');
	N = conv2(ones(size(A)),b','same');
	C = C./N;
	%b = b/sum(b);
	%for i=1:size(A,3)
	%	C(:,:,i) = conv2(A(:,:,i),b','same');
	%end
else
	C = A;
end
end