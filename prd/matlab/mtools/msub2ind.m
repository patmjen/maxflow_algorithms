function ii = msub2ind(sz,i1,i2,i3)
% ii = msub2ind(sz,i1,i2,i3)
% returns out of range or NaN indicies as NaN
%

%i1 = double(i1);
%i2 = double(i2);
if(nargin==3)
% normal behaviour
ii = (i2-1)*sz(1)+i1;

% added behaviour:
ii(~(i1>=1 & i2>=1 & i1<=sz(1) & i2<=sz(2))) = nan;

elseif nargin==4
% normal behaviour
ii = (i3-1)*sz(2)*sz(1)+(i2-1)*sz(1)+i1;

% added behaviour:
ii(~(i1>=1 & i2>=1 & i3>=1 & i1<=sz(1) & i2<=sz(2) & i3<=sz(3))) = nan;

end

end