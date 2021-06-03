function x = select(y,i1,varargin)
%
%	x = select(y,i1,...) -- indexing function with extended capabilities
%
switch length(varargin)
    case 0
        x = y(i1);
    case 1
        i2 = varargin{1};
        sz = size(y);
        [ind s1 s2] = combine_index(i1,i2,sz(1:n_dims(i1)),sz(n_dims(i1)+1:n_dims(i1)+n_dims(i2)));
        x = reshape(y(ind),s1,s2);
    otherwise
        error('index combination for 3 and more subindeces is not implemented');
end
end

function [index s1 s2] = combine_index(i1,i2,sz1,sz2)
if(islogical(i1))
    if(islogical(i2))
    else
        i2 = index2logical(i2,sz2);
    end
else
    if(islogical(i2))
        i1 = index2logical(i1,sz1);
    else
        i1 = index2logical(i1,sz1);
        i2 = index2logical(i2,sz2);
    end
end
index = logical(reshape(kron(i1,i2),[sz1 sz2]));
s1 = nnz(i1);
s2 = nnz(i2);
end

function l = index2logical(ind,sz)
l = logical(zeros(1,sz));
l(ind)=1;
end

function n = n_dims(A)
n = nnz(size(A)-1>0)+max(size(A)-1==1);
end
