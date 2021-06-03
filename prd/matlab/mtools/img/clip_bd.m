function I = clip_bd(I,sz)

%if(any(sz)>0)
I = I(1+sz:end-sz,1+sz:end-sz,:);
%end

end