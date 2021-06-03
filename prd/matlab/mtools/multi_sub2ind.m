function ii = mult_sub2ind(ssz, i,j,k)
%
%  computes index ii from multiindex (i,j) for multisize ssz
%

if size(ssz,1)==1% 1D multiindex
	cssz = [0 cumsum(ssz)];
	ii = cssz(j)+i;
elseif size(ssz,1)==2% 2D multiindex
	pssz = ssz(1,:).*ssz(2,:);
	cssz = [0 cumsum(pssz)];
	if nargin==2 || isempty(k)
		k = 1;
	end
	ii = cssz(j)+(i-1).*ssz(1,j)+k;
end

end
