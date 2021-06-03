function B = cumwin(A,W)
% B = cumwin(A,W)
% sums input A over all windows of size W x W
% values outside A are assumed 0

if(size(A,3)>1)
	B = zeros(size(A));
end

for i=1:size(A,3)
	hW = floor(W/2);
	aa = zeros(msize(A,[1 2])+2*hW+1);
	aa(hW+2:end-hW,hW+2:end-hW) = A(:,:,i);
	aa = cumsum(cumsum(aa,1),2);
	b = +aa(1:end-W,1:end-W)-aa(W+1:end,1:end-W)-aa(1:end-W,W+1:end)+aa(W+1:end,W+1:end);
	if(2*hW==W)
		b(1,:)=[];
		b(:,1)=[];
	end
	B(:,:,i) = b;
end
%{
if(length(W)==1)
	W = [W W];
end
B = conv2(A,ones(W),'same');
%}
end