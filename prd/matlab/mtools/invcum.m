function x = invcum(y)
%
%  x inv_cum(y) computes inverse of cumulative sum of y, y must be positive
%

x = accumarray(cumsum(y(:)),1)';
x = cumsum([1 x]);
x(end)=[];

if(size(y,1)>size(y,2))
	x = x';
end

end