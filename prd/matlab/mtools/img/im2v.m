function v = im2v(I)

sz = size(I);
sz(2)=sz(2)*sz(1);sz(1)=[];
if(length(sz)==1)
	sz(end+1)=1;
end
v = transp(reshape(I,sz));

end