function nI =  pad_bg_add(I,addsz,val)
%
%  [nI nx0] =  pad_bg_add(I,addsz) -- adds free space around the image
%

if length(addsz)==1
	addsz = [addsz addsz];
end

if(~exist('val','var')|| isempty(val))
	val = I(1,1,:);
end
if(length(val)==1)
	val = repmat(val,[1 1 size(I,3)]);
end

sz2 = msize(I,[1 2]);
esz  = sz2+addsz*2;

nI = repmat(val,[esz 1]);
nI(1+addsz(1):end-addsz(1),1+addsz(2):end-addsz(2),:) = I;

end