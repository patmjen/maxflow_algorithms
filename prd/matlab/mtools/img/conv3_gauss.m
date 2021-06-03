function C = conv3_gauss(A, sigma)
%
%  C = conv3_gauss(A, sigma)
%		sigma - scalar or 3-vector
%
if(any(sigma>0))
	if length(sigma)==1
		sigma = [sigma sigma sigma];
	end

	for j=1:3
		b{j} = normpdf([0:1:max(2,3*sigma(j))],0,sigma(j));
		b{j} = [fliplr(b{j}(2:end)) b{j}];
		b{j} = b{j}/sum(b{j});
	end

	for i=1:size(A,3)
		C(:,:,i) = conv2(A(:,:,i),b{1}(:),'same');
		C(:,:,i) = conv2(C(:,:,i),b{2}(:)','same');
	end

	C = permute(C,[3 1 2]);
	for i=1:size(C,3)
		C(:,:,i) = conv2(C(:,:,i),b{3}(:),'same');
	end
	C = permute(C,[2 3 1]);
else
	C = A;
end
end