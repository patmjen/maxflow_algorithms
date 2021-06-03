function B = write(A,i1,varargin)
%
%		B = write(A,i1,i2..,in, val)
%
if isempty(i1)
	B = A;
	return;
end

switch length(varargin)
	case 0
        error('no val');
	case 1
		val = varargin{1};
		B = A(:);
		B(sub2ind(size(A),i1)) = val;
		B = reshape(B,size(A));
	case 2
		i2 = varargin{1};
		val = varargin{2};
		B = A(:);
		B(sub2ind(size(A),i1,i2)) = val;
		B = reshape(B,size(A));
	case 3
		i2 = varargin{1};
		i3 = varargin{2};
		val = varargin{3};
		B = A(:);
		B(sub2ind(size(A),i1,i2,i3)) = val;
		B = reshape(B,size(A));
    otherwise
        error('selectiong for 4 and more subindeces is not implemented');
end

end