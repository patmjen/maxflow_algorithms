function C = conv2_sep(A, varargin)
%
%  C = conv2_sep(A, g) - convolve A with (g(:)*g(:)')
%  C = conv2_sep(A, f,g) - convolve A with (f(:)*g(:)')
%
if nargin==1
g = varargin{1};
g = fliplr(g(:)');
for i=1:size(A,3)
	C(:,:,i) = conv2same(A(:,:,i),g(:));
	C(:,:,i) = conv2same(C(:,:,i),g(:)');
end
else
f = varargin{1};
g = varargin{2};
f = fliplr(f(:)');
g = fliplr(g(:)');
for i=1:size(A,3)
	C(:,:,i) = conv2same(A(:,:,i),f(:));
	C(:,:,i) = conv2same(C(:,:,i),g(:)');
end
end

end