function [c] = eci(X,alpha)
if(~exist('alpha','var'))
	alpha = .05;
end
X = X(:);
[f,x] = ecdf(X);
err = 0;
i1 = 2;
i2 = length(x);
while err < alpha
	if (f(i1+1)-f(i1))/(x(i1+1)-x(i1)) < (f(i2)-f(i2-1))/(x(i2)-x(i2-1))
		i1 = i1+1;
	else
		i2 = i2-1;
	end
	err = f(i1)+1-f(i2);
end
%i1 = max(1,find(f>(1-alpha)/2,1,'first')-1);
%i2 = min(find(f<1-(1-alpha)/2,1,'last')+1,length(x));
c(1) = x(i1);
c(2) = x(i2);
%n = length(x);
%
% min u-l
%  u>l
%
%
end