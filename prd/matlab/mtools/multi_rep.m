function y = multi_rep(x,n)
% 
%  y = multi_rep(x,n) repats each value in x the number of times specified by n
%

y = x(invcum(n));

end