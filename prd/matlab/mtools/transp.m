function A = transp(A,dim12)
% A = transp(A,dim12) - flips dimensions dim12(1) and dim12(2), default is dim12=[1 2]
%
if ~exist('dim12','var') || isempty(dim12)
	dim12 = [1 2];  % default to permute dims 1,2
end
p = 1:ndims(A);
p(dim12(1))=dim12(2);
p(dim12(2))=dim12(1);
A = permute(A,p);

end