function B = mind(A,varargin)
% B = mind(A,ii)
% B = mind(A,i,j)
% B = mind(A,i,j,k)
% returns A(ii) resp. A(i,j) resp. A(i,j,k) and puts NaNs where one or more indicies is NaN
%

if nargin == 2
	i = varargin{1};
	m = ~isnan(i);
	B = nan(size(i));
	B(m)=A(i(m));
elseif nargin == 3
	i = varargin{1};
	j = varargin{2};
	ii = msub2ind(size(A),i,j);
	B = nan(size(ii));
	mask = ~isnan(ii);
	B(mask)=A(ii(mask));
elseif nargin == 4
	i = varargin{1};
	j = varargin{2};
	k = varargin{3};
	ii = msub2ind(size(A),i,j,k);
	B = nan(size(ii));
	mask = ~isnan(ii);
	B(mask)=A(ii(mask));
end

end 