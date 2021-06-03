function A = msparse(i,j,s,varargin)
% 
% A = msparse(i,j,s,varargin)
% allows nans in i,j,s and and such elements are ommited
%

m = ~(isnan(i) | isnan(j) | isnan(s));

A = sparse(i(m),j(m),s(m),varargin{:});

end