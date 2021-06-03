function hh = hist2i(I1,n1,I2,n2)

%ii = [I1(:)' I2(:)'];
%[b m n] = unique(ii,'rows');
hh = zeros(n1,n2);
ii = sub2ind(size(hh),I1,I2);
hh = full(sparse(double(I1(:)),double(I2(:)),ones(length(I1(:)),1),n1,n2));
end