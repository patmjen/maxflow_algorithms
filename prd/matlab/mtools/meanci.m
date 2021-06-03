function [m ci] = meanci(X,dim)
if(~exist('dim','var'))
	g = ones(size(X));
else
	if dim==1
		g = repmat([1:size(X,2)],size(X,1),1);
	elseif dim==2
		g = repmat([1:size(X,1)]',1,size(X,2));
	end
end
	[m ci] = grpstats(X,g,{'mean','meanci'});
end