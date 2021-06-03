function x = finvert(y)
%
% x = inv_mon(y) inverts discrete monotonouse non-decreasing function y
%
% Input must satisfy: y(1) = 1, y(i+1)>=y(i), y(i_max) = x_max
%
% Output satisfies: x(1)=1, x(y(i):y(i+1)-1) = i, i = 1:length(y)-1, x(y_max) = i_max

x = accumarray(y(:),1)';
x = cumsum([1 x]);
x(end)=[];

end