function sz = msize(x,dims)
	a = size(x);
	if(~exist('dims','var') || isempty(dims) )
		sz = a;
	else
		sz = ones(1,length(dims));
		ii = dims<=length(a);
		sz(ii) = a(dims(ii));
	end
end