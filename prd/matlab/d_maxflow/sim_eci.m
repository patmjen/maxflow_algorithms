function [y c] = sim_eci(X,alpha)
if(~exist('alpha','var'))
	alpha = .05;
end
X = X(:);
y = mean(X);
d = abs(X-y);
n = length(X);
d = sort(d,'ascend');
%cs = cumsum(d);
%i = find()
i =1;
while sum(d<=d(i)) < n*(1-alpha)
	i = i+1;
end

c(1) = y-d(i);
c(2) = y+d(i);

end