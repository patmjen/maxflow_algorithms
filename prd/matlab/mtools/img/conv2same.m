function C = conv2same(A,B)
	C = conv2(A,B,'same');
	%N = conv2(ones(size(A)),B,'same');
	%C = C./max(N,1e-20);
end