function B = meanwin(A,W)
% B = meanwin(A,W)
% averages over windows of size W

B = cumwin(A,W);
nn = cumwin(ones(imsize(A)),W);
B = B./repmat(nn,[1 1 size(A,3)]);

end